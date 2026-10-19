# SedSAT3 — Reviewer Test Plan (code-level, GUI-driven)

**Purpose:** things a *code* reviewer can test by operating the software, without
judging scientific validity (that is the domain reviewer's job). Each test is
either a robustness/consistency check or verifies the software respects its own
stated behavior. Findings from these are citable software observations.

**Scope note / how to run:** SedSAT3 has no CLI or scriptable entry point —
every analysis is triggered through the Qt GUI (`genericform` builds a command
+ argument map and calls `MainWindow::Execute()`, which dispatches to
`Conductor::Execute()`). So these are manual GUI steps. Use
`examples/SampleData.xlsx` unless noted. Launch via `pixi run run-app`.

**Where the methods live (from `resources/tools.json`):**
- Deterministic fingerprinting (Levenberg–Marquardt): menu
  **Deterministic Sediment Fingerprinting → 5-Fingerprinting**
- Bayesian fingerprinting (this is the MCMC sampler, `ExecuteCMBBayesian`): menu
  **Bayesian Sediment Fingerprinting → 5-Bayesian Chemical Mass Balance Analysis**
- Note: the genetic-algorithm commands exist in `Conductor` but are **not
  exposed in `tools.json`** — see Test 6.

---

## Test 1 — Determinism / reproducibility (HIGH VALUE; finding expected)

**Why:** the Bayesian (MCMC) sampler seeds its RNG from the wall clock:
`srand(int(time(NULL)) ^ omp_get_thread_num() + kk)` (`include/MCMC/MCMC.hpp`
line ~433). There is no user-facing seed control. Repeated runs on identical
input should therefore differ, with no way to reproduce a specific result.

**Steps:**
1. Import `SampleData.xlsx`.
2. Run **Bayesian Chemical Mass Balance Analysis** with default settings; record
   the source contribution estimates (and credible intervals).
3. Without changing anything, run it again. Record again.
4. Compare.

**What to note:** Do the two runs differ? By how much (trivial vs. material)?
Is there *any* setting to fix a random seed? A code reviewer can legitimately
recommend adding a seed option for reproducibility. (Bonus code smell to
mention: seeding via `srand(... ^ omp_get_thread_num())` inside an OpenMP
parallel region makes results depend on thread timing.)

**Also try:** the deterministic **Levenberg–Marquardt** run twice — it should be
*identical* both times (no RNG). If it is, that's a nice contrast to report; if
it is not, that is a more serious finding.

## Test 2 — Output obeys the model's own constraints (HIGH VALUE)

**Why:** the code enforces sum-to-one structurally — it solves for N−1 source
contributions and derives the last "from the sum constraint"
(`src/sourcesinkdata.cpp` ~lines 702–715). So results should be non-negative
and sum to ~100%.

**Steps:**
1. Run any fingerprinting method on the sample data.
2. From the results table, add up the source contributions.

**What to note:** Do they sum to ~1.0 (100%)? Are any negative? The
derived-by-subtraction source (the "last" one) is where a violation would
surface — a negative there is a citable finding. This is arithmetic, not
science.

## Test 3 — Cross-method consistency (MEDIUM VALUE)

**Why:** same input, different solver. You are checking *consistency*, not
which is "right."

**Steps:**
1. Run **Levenberg–Marquardt** and **Bayesian CMB** on the same sample data.
2. Compare the source contribution estimates.

**What to note:** Do the two methods land in the same ballpark, or diverge
sharply? Either way, report it as a factual observation for the domain
reviewer. Rough agreement is a reassuring smoke test; sharp divergence is worth
flagging.

## Test 4 — Input robustness / graceful failure (MEDIUM VALUE; don't rabbit-hole)

**Why:** a code reviewer legitimately assesses error handling. Note the code has
an input guard (`Conductor::CheckNegativeElements`) — Test 4c probes it.

**Steps (make a throwaway copy of the xlsx for each):**
- 4a. Delete a column the docs say is required (e.g., `D50` or `TOC`); import.
- 4b. Put a non-numeric string in a tracer cell; import and run a method.
- 4c. Introduce a negative concentration value; run a method (should be caught).
- 4d. Reduce a source sheet to a single sample; run a method.

**What to note:** For each — clear error message, silent wrong answer, or crash?
Clear errors = good. Crash or silent-garbage = finding. One or two of these is
plenty; don't exhaustively fuzz.

## Test 5 — Output round-trips honestly (LOW–MEDIUM VALUE)

**Steps:**
1. Run a method; export results to table and note key numbers.
2. Export/save the plot; confirm plotted values match the table.
3. Save the project, close, reopen; confirm it restores to the same state.

**What to note:** Any mismatch between table, plot, and exported files; any loss
of state on reload.

## Test 6 — Advertised vs. actual methods (DOC/CODE CONSISTENCY; finding likely)

**Why:** `README.md` advertises three apportionment methods (Levenberg–Marquardt,
genetic algorithm, Bayesian MCMC). But `resources/tools.json` exposes only
Levenberg–Marquardt and Bayesian CMB in the menus; the **GA commands
(`"GA"`, `"GA (fixed elemental contribution)"`, `"GA (disregarding targets)"`)
exist in `Conductor` but are not on any menu.**

**Steps:**
1. Look through the GUI menus for any genetic-algorithm option.

**What to note:** If the GA is genuinely unreachable from the UI, that's a
README-vs-implementation mismatch worth reporting (either wire it up, or adjust
the README's claim).

---

## What to hand off to the domain reviewer (out of code-review scope)

- Whether the contribution *values* are scientifically plausible.
- Whether the priors/likelihood in the Bayesian method are appropriate.
- Whether tracer selection / DFA choices are defensible.
- Whether the methods are the right ones for the science.

## Verified code facts underpinning this plan (citable)

- Clock-based RNG seeding, no seed control: `include/MCMC/MCMC.hpp:~433`,
  `Utilities/TimeSeries.hpp:~443`.
- Sum-to-one enforced by construction: `src/sourcesinkdata.cpp:~702–715`.
- Method dispatch table: `src/conductor.cpp` (`Conductor::Execute`).
- User-facing menu map: `resources/tools.json` (GA absent).
- No CLI entry point: `main.cpp` only launches the Qt window.


---

## Test 7 — Thread-safety of the RNG (SUGGESTIVE ONLY — primarily a code finding)

**Why this is different from the other tests:** the non-thread-safe use of
global `rand()` inside the OpenMP parallel region (see Section 11 of
`CODE_REVIEW_OBSERVATIONS.md`) is a **code-inspection finding**, not something
black-box testing can reliably prove. It is timing-dependent and is *masked* by
the separate non-reproducibility issue (degraded randomness looks like normal
run-to-run variation when the seed is already random). Report it based on the
source, not on a reproduced failure.

**What it does at runtime (for your understanding):** it will **not crash**.
The likely effect is silent statistical degradation of the random stream; the
program runs to completion and produces plausible-looking output. This is the
"causes a problem but silently continues" category.

**Optional suggestive check (do not present as proof):**
1. In the Bayesian/MCMC settings, set number of threads = 1; run on sample data;
   record results.
2. Set number of threads > 1 (e.g., 4); run again on the same data; record.
3. Note whether multi-thread behavior looks systematically different from
   single-thread beyond expected sampling noise.

**What to note:** any difference is *suggestive only* — you cannot cleanly
separate a threading effect from the clock-seed randomness. The citable finding
remains the code inspection: global `rand()`/`srand()` called from within
`#pragma omp parallel` (`include/MCMC/MCMC.hpp` ~lines 429–440).
