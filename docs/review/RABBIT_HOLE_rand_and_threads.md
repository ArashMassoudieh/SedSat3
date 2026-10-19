# Down Memory Lane: what *really* happens when two threads call `rand()`

*A side-trip that started from a reproducibility finding in the SedSAT3 code
review and wandered all the way down to cache-coherence hardware. Written up
because it's easy, as we pile on abstractions, to lose sight of what the machine
is actually doing under the hood.*

---

## Where this came from

The SedSAT3 MCMC sampler runs its Markov chains in parallel with OpenMP, and
inside that parallel region each chain draws random numbers through C's global
`rand()` (via `getstdnormalrand()` → `unitrandom()` in
`Utilities/NormalDist.cpp`). The code review flagged this as a
**reproducibility** problem (no seed control, §10) *and* a **thread-safety**
problem (`rand()` called from an OpenMP parallel region, §11). This document is
the "why," dug out layer by layer.

The guiding question evolved like this:

1. Would a pinned random seed make the MCMC reproducible? *(No.)*
2. Why not? *(No seed control **and** `rand()` in a parallel region.)*
3. What's actually wrong with `rand()` in a parallel region? *(A data race.)*
4. What does that race do at a low level? *(This document.)*

---

## Layer 0 — Why a PRNG must keep state at all

`rand()` is a **pseudo**-random generator: a deterministic function that emits a
sequence that merely *looks* random. To produce the *next* value it must
remember where it is in the sequence. That "where am I" is a single running
value — the state (a.k.a. the seed).

The classic textbook implementation is a **Linear Congruential Generator (LCG)**,
essentially what the C standard shows as a sample and close to what glibc's
simplest mode and MSVC use:

```c
static unsigned long next = 1;   // <-- the process-global state

int rand(void) {
    next = next * 1103515245 + 12345;   // read next, compute, write next back
    return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) {
    next = seed;                  // "seeding" == assigning to that one global
}
```

Two things fall out immediately:

- The single statement `next = next * A + C` is a **read-modify-write** of a
  shared global.
- `srand()` is *nothing but* a write to that same global. This is why a
  per-thread `srand(... ^ omp_get_thread_num())` (as attempted in the SedSAT3
  code) cannot do what it looks like — every thread is scribbling on the *same*
  `next`.

Single-threaded, this is flawless: calls are strictly ordered
(read-write-read-write…), the state threads cleanly through each call, and the
output is perfectly deterministic given a seed. `rand()` survived for decades
because it was designed in a single-threaded world.

---

## Layer 1 — The operation in machine terms

`next = next * 1103515245 + 12345;` is not one action to the CPU. It's roughly:

```
load   next        -> register     ; READ memory into a core-private register
imul   register, 1103515245        ; compute (entirely inside the core)
add    register, 12345
store  register    -> next         ; WRITE the register back to memory
```

The multiply/add happen in **registers**, which are private to each core. The
only shared thing is the memory location `next`. So all contention is about the
`load` and the `store`.

---

## Layer 2 — What keeps it from "blowing up": per-access atomicity

The first hardware guarantee: **an aligned, word-sized memory access is atomic**
on mainstream CPUs. On x86-64, reading or writing an aligned 4- or 8-byte
integer is a single, indivisible memory transaction. The memory system will not
interleave two writes at the *bit* level.

Consequence: if thread A stores `0x1111` and thread B stores `0x2222` to the
same location, the result is either `0x1111` or `0x2222` — one of them, whole.
You can **never** get `0x1211` (a "torn" value, half of each).

That single fact is the whole reason this code doesn't crash. The value in
memory is always a complete, legal integer that *some* thread wrote — never a
trap value, corrupted pointer, or invalid state.

> Caveat: this guarantee is specifically for **aligned, word-sized** data
> (like `rand`'s `int`/`long`). Misaligned or larger-than-word data *can* tear.
> And formally, per the C++ memory model, this is still **undefined behavior** —
> "no catastrophe" is a property of how real CPUs implement word-sized access,
> not a promise the language makes. The airtight way to state it: *UB whose
> realistic manifestation on these platforms is statistical, not a crash.*

---

## Layer 3 — What the hardware does NOT prevent: the lost update

Atomicity applies to *each* load and *each* store individually — but **not to
the load-compute-store sequence as a group**. That grouping is where the race
lives:

```
Thread A: load next (=100) ─┐
Thread B: load next (=100) ─┤ both read 100 before either stores
Thread A: compute -> 205    │
Thread B: compute -> 205    │
Thread A: store 205 ────────┘
Thread B: store 205   <- clobbers A's store with the same 205
```

Every individual load and store here is atomic and clean. But B read the state
*before* A wrote it, so B computed from stale data. The net effects:

- **Lost update** — the sequence advanced once where it should have advanced
  twice.
- **Duplicated draw** — two chains just received the *same* "random" number.
- **Reordered consumption** — which thread gets which value depends on
  scheduling, so the draw→chain mapping is nondeterministic run to run.

The memory location holds a perfectly valid `205`. Nothing illegal happened from
the memory system's view: two legal reads, two legal writes. The damage is to
the **bookkeeping**, not the value.

And note the collision window is *not* "two calls at precisely the same
instant." It's the entire read-modify-write span: any time thread B reads
`next` after A read it but before A wrote back, they collide. Under a tight
parallel loop hammering `rand()`, that's routine, not miraculous.

---

## Layer 4 — Why the steps aren't forced to stay together

To make a read-modify-write indivisible you need explicit synchronization, and
`rand()` uses none:

- a **lock/mutex** around the sequence (correct but serializes threads — slow), or
- an **atomic read-modify-write instruction** — on x86, `lock cmpxchg`
  (compare-and-swap) or `lock xadd`, where the `lock` prefix makes the CPU hold
  exclusive ownership of the cache line for the *entire* operation so no other
  core can slip in.

Plain `rand()` emits ordinary `load`/`imul`/`store` — **no `lock` prefix, no
fence**. So the hardware treats the three steps as unrelated,
independently-atomic operations. That's deliberate: locking every `rand()` call
would penalize the overwhelmingly common single-threaded use for no benefit.

This is also why the tooling names it: the review's `.clang-tidy` config invokes
`concurrency-mt-unsafe`, the check specifically for functions like `rand()` that
are *documented as not thread-safe* because of exactly this shared global state.
The C standard and POSIX do not require `rand()` to be thread-safe.

---

## Layer 5 — Cache coherence: why it settles to a sane value

One more layer underneath, which is what keeps the cores from *permanently*
disagreeing. Multicore CPUs run a **cache-coherence protocol** (MESI and
relatives). Each core caches `next` in its own L1, but a **write** requires
exclusive ownership of that cache line:

- To store to `next`, a core must acquire the line in Modified/Exclusive state,
  which **invalidates every other core's copy**.
- So writes to a single location are **serialized** by the protocol — they
  happen in *some* global order, one at a time, and every core eventually
  converges on the same final value.

What coherence does **not** do is prevent the *logical* lost update. It orders
the writes, but if a write was computed from stale data, coherence faithfully
commits that stale-derived value anyway. **Coherence guarantees a consistent
value, not a correct sequence.**

---

## Putting it together

Three layers, in order:

1. **Aligned word-sized access is atomic** → no torn values → the stored integer
   is always whole and legal → no crash, no corruption.
2. **Cache coherence serializes writes** → all cores converge on one consistent
   final value → no permanent divergence.
3. **But nothing makes load-compute-store atomic as a unit** → updates based on
   stale reads are silently lost and draws get duplicated → *this* is the RNG
   degradation, and it's the one thing the hardware can't prevent without
   explicit synchronization the code doesn't use.

So: **no catastrophe, valid-but-mis-sequenced values, and a resulting erosion of
the RNG's designed statistical behavior.** The generator's proven properties
(period, uniformity, low serial correlation) hold for the *sequential*
read-write-read-write pattern; concurrent access voids that analysis.

---

## Why this matters for the review (and the honest limits)

- **For reproducibility:** the race alone is *sufficient* to break bit-level
  repeatability — which is why pinning a seed (§10) would not be enough; you'd
  also need each thread to own its generator. This is the concrete, defensible
  use of the finding.
- **For statistical quality:** report it as a *possible* degradation, explicitly
  a **code-inspection finding, not a runtime-observed one**. It's *masked* by the
  unseeded clock (§11): when the seed is already random every run, a
  slightly-correlated stream is statistically indistinguishable from a clean one
  seeded differently. So:
  - **Assertable:** the code contains a data race on the RNG state that
    *permits* lost updates and duplicated draws, which *can* degrade the stream.
  - **Not assertable:** that the results *were* measurably degraded, or by how
    much. The magnitude is unquantified and, on this code, not readily
    observable.

The reason to fix it isn't a demonstrated harm — it's removing an uncontrolled,
unquantifiable variable from a scientific computation for essentially no cost.

---

## The fix (resolves the race *and* reproducibility in one move)

Give each thread/chain its **own** PRNG object, each seeded deterministically
from a single master seed. Modern generators keep their state in an object *you*
own rather than a hidden global:

```cpp
std::mt19937 gen(master_seed + chain_index);   // state lives in 'gen', not a global
std::normal_distribution<double> dist(0.0, 1.0);
double x = dist(gen);                           // touches only gen's own state
```

- **No shared memory between threads** → the data race disappears by
  construction, not by locking.
- **Deterministic per-chain streams** → reproducible regardless of thread
  scheduling (once the master seed is also *recorded* in the output).
- **No cross-chain correlation** from clobbered draws.

`gsl_rng` (GSL is already a dependency) works equally well. Either way, the move
is from *"one hidden global that all threads fight over"* to *"one private state
per thread"* — which is precisely why it closes §10 and §11 together.

---

## TL;DR

By design, every `rand()` call **writes** its updated state back to a single
process-global variable, and `srand()` just sets that global. Single-threaded
that's flawless; called from an OpenMP parallel region it's a genuine **data
race**. The hardware's per-access atomicity and cache coherence keep it from
crashing (values stay whole and coherent), but nothing protects the
three-step read-modify-write as a unit — so you get **lost updates and
duplicated draws**: harmless to program stability, corrosive to the RNG's
intended statistical quality, and fatal to reproducibility. The clean fix is a
per-thread PRNG seeded from one recorded master seed.
