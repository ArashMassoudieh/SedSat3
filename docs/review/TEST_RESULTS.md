# SedSAT3 — Manual GUI Test Results (code-review evidence)

**Reviewer:** USGS software code review
**Date:** 2026-09-15
**Build:** Windows/pixi/MSVC build (see `BUILDING_ON_WINDOWS_WITH_PIXI.md`)
**Data:** `examples/SampleData.xlsx`, all sample groups imported
(Bank, Crop, Forest, Mixture, Pasture)

> **Scope reminder.** These are *behavioral / tooling* observations a code
> reviewer can make by operating the software. They deliberately do **not**
> judge scientific validity of the apportionment values — that is the domain
> review's job. Where a result raises a scientific question, it is flagged as a
> handoff, not answered here.

---

## Preprocessing note — negative-value guard (context for the runs below)

Running a Bayesian CMB analysis immediately after import triggered:

> "There are zero or negative values for element 'C13' in sample group 'Bank'"
> (and multiple similar instances).

**Interpretation (code-review-legitimate):** the CMB likelihood operates in
log-space, so elements with ≤0 values are rejected by `Conductor::CheckNegativeElements`
(`src/conductor.cpp:~198`). `C13` is a stable-isotope ratio (δ¹³C), which is
*legitimately* negative — so this is not corrupt data but a real tracer the
log-based likelihood cannot accept as-is. To proceed, the offending
elements/isotopic tracers were de-selected in the tracer-selection step (README
quickstart step 4, which is required before running an analysis).

**Observation for the record:** the negative-value guard does not distinguish an
*erroneous* negative from a *legitimately-negative isotopic tracer*, and offers
no transform path for the latter. (Same guard carries the missing-return bug in
§12a; same log-space assumption as the `log(variance_term)` risk in §12a and the
Box-Cox code.) Whether δ¹³C *should* be usable as a tracer is a domain question.

---

## Test 1 — Determinism / reproducibility (RESULT: non-reproducible, as predicted)

**Procedure:** ran **Bayesian Chemical Mass Balance Analysis** repeatedly on the
same input, unchanged between runs. Recorded the *Source Contribution Credible
Interval — maximum (upper) bound* for the **Bank** source.

**Round A — default sample count (~500):**

| Run | Bank upper-CI bound |
| --- | ------------------- |
| 1   | 0.4656              |
| 2   | 0.6362              |
| 3   | 0.6722              |

Spread ≈ **0.21**. Similar run-to-run variation observed across all samples.

**Round B — sample count increased to 3000:**

| Run | Bank upper-CI bound |
| --- | ------------------- |
| 1   | 0.4787              |
| 2   | 0.5846              |
| 3   | 0.4632              |

Spread ≈ **0.12**.

**Code-review findings (in scope, airtight):**

1. **Non-reproducibility confirmed (Test 1).** Identical inputs produce
   materially different outputs; there is **no seed control and no recorded
   seed**, so a specific result cannot be reproduced. This empirically confirms
   the source-level finding in `CODE_REVIEW_OBSERVATIONS.md` §10
   (clock-seeded RNG, `srand(time(NULL)...)`, no seed exposed).
2. **Convergence behavior (factual).** Increasing samples ~500→3000 reduced the
   run-to-run spread on the Bank upper-CI bound from ~0.21 to ~0.12 — reduced
   **but not eliminated**. The posterior summaries had not stabilized at the
   sample counts the GUI readily allows. (Direction is consistent with normal
   MCMC sampling-noise shrinkage; the *residual* spread is what remains open.)
3. **Missing convergence diagnostics (tooling gap).** The software surfaces no
   convergence diagnostics (e.g. R-hat / Gelman-Rubin, effective sample size,
   trace or autocorrelation plots) that would let a user judge whether a run has
   converged. Combined with the absent seed, a domain reviewer inherits the tool
   without the instruments needed to assess or reproduce a run.
   **Note:** the *capability* is partly present in the code — `MCMC.h` runs
   multiple chains with burn-in and its doc comments cite the Gelman-Rubin
   statistic — but no such diagnostic is **surfaced to the user** in the GUI. So
   the gap is exposure/reporting, not total absence of the concept in the code.

**What the MCMC is (for the record):** a self-implemented **adaptive random-walk
Metropolis** sampler over a Bayesian posterior (`posterior()` = log-likelihood of
observed vs. modeled target profile + configurable per-parameter log-priors:
`normal` / `uniform` / `log-normal`), with a self-tuning proposal width
(target `acceptance_rate`), multiple chains, and burn-in
(`include/MCMC/MCMC.hpp`). Original author code, not a third-party MCMC library.

**Handoff to domain review (explicitly NOT a code-review conclusion):** whether
the residual ~0.12 spread at 3000 samples reflects acceptable sampling noise or
under-convergence is a statistical/domain question (burn-in adequacy, chain
count, autocorrelation, sufficient iterations). It is **not** asserted here that
the method is flawed — only that the output has not stabilized and the tool
provides no means to assess convergence. Two data points (500 and 3000) are
provided above to support that assessment.

**[WESTENBROEK NOTE: that conclusion is a little strong...]**

---

## Deterministic Levenberg–Marquardt contrast (RESULT: reproduces exactly)

Ran the deterministic **Levenberg–Marquardt** method
(**Deterministic Sediment Fingerprinting → 5-Fingerprinting**) multiple times on
the same input and settings as the MCMC runs above. LM output is a **point
estimate** of source contributions (`GetContribution()`), not an interval.

**Result — identical every run.** Source contribution point estimates:

| Source  | Contribution |
| ------- | ------------ |
| Bank    | 0.240        |
| Crop    | 0.198        |
| Forest  | 0.246        |
| Pasture | 0.315        |

(Same values on every repeat run.)

**Findings (in scope):**

1. **Deterministic method is exactly reproducible** — as expected from the
   source: `SolveLevenberg_Marquardt` has no RNG (the `(JᵀJ + λ·diag)⁻¹`
   iteration). This is the **clean contrast to the MCMC**: it demonstrates the
   run-to-run variation in Test 1 is **specific to the stochastic/RNG path**
   (the unseeded clock-based RNG, §10), not general nondeterminism in the
   software. Strengthens the Test 1 / §10 finding.
2. **Incidental sum-to-one check (Test 2, deterministic method).** The four
   contributions sum to 0.240 + 0.198 + 0.246 + 0.315 = **0.999 ≈ 1.0**, and all
   are **non-negative** — consistent with the sum-to-one-by-construction design
   (`src/sourcesinkdata.cpp` ~702–715). No constraint violation observed for the
   deterministic method on this dataset. *(Test 2 for the Bayesian method — and
   checking the derived-by-subtraction "last" source for any negative — remains
   to be recorded.)*

---

## Test 6 — Advertised vs. actual methods (RESULT: GA absent from GUI, confirmed)

**Procedure:** inspected the SedSAT3 GUI menus for any genetic-algorithm option.

**Result:** **nothing explicitly mentioning genetic algorithms appears anywhere
in the GUI.** The GUI exposes exactly three Tools folders — **Bayesian Sediment
Fingerprinting**, **Deterministic Sediment Fingerprinting**, and **Other
Statistical Tools** — and none of the three contains any genetic-algorithm
entry (Other Statistical Tools holds preprocessing/stats only: OM-size
correction, MLR, DFA family, KS, distribution fitting, correlation/covariance).

**Finding (README-vs-implementation mismatch, now confirmed by both code and
GUI):**

- `README.md` and `code.json` advertise **three** apportionment methods:
  deterministic (Levenberg–Marquardt), **genetic algorithm**, and Bayesian MCMC.
- The GA commands (`"GA"`, `"GA (fixed elemental contribution)"`,
  `"GA (disregarding targets)"`) exist in `Conductor` but are **not exposed in
  `resources/tools.json`**, so they reach no menu.
- Empirically confirmed here: the GA is **unreachable from the UI**.
- Corroborated further by the orphaned-GA code findings: `GA_impl` stubs
  (§9), and the missing `StringOP.h` include that makes `GA.cpp` non-compilable
  as-is (§12d) — i.e., the GA path is not merely unexposed but appears
  **not built / dead**.

**Recommendation:** reconcile the advertised feature set with reality — either
wire up (and repair) the GA and expose it in `tools.json`, or remove the
genetic-algorithm claims from `README.md`/`code.json`.

---

## Test 5 — Output / project round-trip (RESULT: persistence passes)

**Procedure:** saved the project, fully shut down the application, reopened it,
and reloaded the saved project.

**Result — state preserved.** The reloaded project retained:

- the imported dataset,
- the **tracer/element exclusions** (the de-selected ≤0/isotopic elements), and
- the **previous session's MCMC results**.

**Finding:** project persistence round-trips correctly on this build — a
reassuring result (state loss on reload would have been a real finding). No
mismatch observed between saved and reloaded state.

*Not separately exercised:* table-vs-plot numeric agreement and file export
fidelity (the other Test 5 sub-checks) — persistence was the highest-value
sub-check and it passes.

---

## Test 4 — Input robustness / graceful failure (RESULT: mostly graceful; one minor finding)

### 4b — non-numeric value in a tracer cell

**Procedure:** edited a copy of `SampleData.xlsx` to place the string `NaN` in a
tracer cell, then imported.

**Result — clear, located rejection (good bucket).** Import refused to load with:

> "Non-numerical value found, sheet: bank, Row: 5, Column: 14, Value: '31NaN50'"

Graceful failure with sheet/row/column coordinates **and an accurate echo of the
offending cell content** — the reviewer had literally entered `31NaN50` into the
cell, and the importer reported exactly that. No silent acceptance, no crash,
and the error message is precise (correct sheet/row/column and verbatim value).
Import validation works well. *(An earlier draft of this note mis-read the
reported value as a concatenation bug; that was incorrect — the value shown was
the exact user input.)*

**Second observation — bare `NaN` takes a different path (detected and
"ignored", not silently poisoning the math).** Entering `NaN` *alone* in a
Calcium cell did **not** trigger the "Non-numerical value" rejection above;
instead a runtime message appeared: **"Ignored NaN, Inf, or -Inf value."**

Partial root-cause (verified) + open question (honest scope):

- **Verified in source (`mainwindow.cpp:699–700`):** the first cell-validity
  check is `QString::toDouble(&isNumber)`, and Qt's `toDouble()` accepts `"NaN"`
  / `"inf"` as valid IEEE-754 values (returns `isNumber = true`). So bare `NaN`
  passes *that* check, which is why it doesn't hit the "Non-numerical value"
  path that `31NaN50` (an invalid float) does.
- **Observed at runtime:** a *second* layer then detects the NaN/Inf and
  "ignores" it — so NaN does **not** silently propagate into the computation.
  This is better than the worst case (silent garbage), and corrects an earlier
  overstatement in this reviewer's analysis.
- **Not located this session:** the exact source of the "Ignored NaN, Inf, or
  -Inf value" message could not be found by string search in the checked-out
  tree (SedSAT3, Utilities, QXlsx, qcustomplot) — it may be assembled from
  fragments at runtime or live in an unscanned template/dependency. Its precise
  handling was therefore not confirmed from source.

**Open behavioral question for follow-up (the real finding):** *what does
"ignored" do to that data point* — drop the single cell, drop the whole sample,
or treat it as missing — and does the subsequent analysis then **proceed
silently on fewer data than the user believes**? "Ignored" with only a
transient message, followed by a normal-looking run, is the pattern worth
checking: it is not silent garbage, but it may be silent data reduction. This is
a bounded follow-up (read the handler once located, or observe whether row/
sample counts change after import).

**Net for 4b:** the ordinary non-numeric path (`31NaN50`) is handled well
(clear, located, accurate rejection). The `NaN`/`Inf` path is *detected and
reported*, not silently corrupting — but the disposition of the ignored value
warrants a short follow-up to confirm it does not quietly shrink the dataset.

### 4c — negative concentration values (guard exercised by the example data itself)

**Procedure:** no edit needed — `examples/SampleData.xlsx` ships with **all
values for element `C13` (δ¹³C) negative**. Running an analysis on the raw
import triggers the guard.

**Result — guard correctly rejects (good bucket).** `Conductor::CheckNegativeElements`
(`src/conductor.cpp:~198`) fires with the popup "There are zero or negative
values for element 'C13' in sample group 'Bank'" and blocks the analysis
(`return false`). The rejection path works.

**Ties to the §12a missing-return finding (empirical characterization).** The
guard's **reject** path (negatives present → `return false`) demonstrably works.
The guard's **clean** path (no negatives → should `return true`) is the one
carrying the missing-return bug (`conductor.cpp:213` falls off the end). In
practice that clean path also *worked* here — the LM/MCMC runs on validly
de-selected/positive tracers proceeded — meaning the missing return currently
yields a truthy value **by luck on this MSVC build**. So both paths behave
correctly *on this build* despite the defect: the bug is **masked, not benign** —
undefined behavior that is not guaranteed to hold across compilers/optimization
levels. This is the most precise statement of the §12a finding.

*(Probes 4a — delete required column — and 4d — single-sample source — were not
run; per the test plan, two probes is sufficient and 4d blurs into domain
territory.)*

---

## Test 4 addendum — negative-value guard fires on *corrected* values, not raw input (finding)

**Observed:** after de-selecting the all-negative isotopic tracers (C13/δ¹³C,
N15/δ¹⁵N), running Levenberg–Marquardt then complained of "zero or negative
values" for **Cobalt, Manganese, 210Pbxs, and Cs137** — even though the **raw
input values for those constituents look fine (positive).**

**Root cause (confirmed in source):** the guard runs on the **corrected**
dataset, not the raw import. `ExecuteLevenbergMarquardt` (`conductor.cpp:~236`)
calls `CreateCorrectedDataset(...)` **first**, then `CheckNegativeElements(...)`.
The OM/size correction applies a **regression-based additive adjustment**
(`elemental_profile.cpp:913`):

```cpp
corrected_profile[element_name] += (reference - measured) * coefficients[1];
```

When `measured > reference` and the regression slope (`coefficients[1]`) is
positive, this term is **negative** and can drive a positive raw concentration
**below zero**. (A multiplicative branch at line 916 stays positive, but the
additive branch does not.) So the reviewer's hypothesis is correct: the
negativity is **introduced by the correction step**, not present in the data.

**Finding (code-review scope):** the guard's diagnostic is **misleading** — it
reports "negative values for element X" as if the *input* were bad, with no
indication that the negativity was **produced by the OM/size correction**. A
user looking at clean positive raw data is left confused (as happened here).
The message should distinguish "negative in input" from "negative after
correction," and ideally point at the correction as the cause.

**Handoff to domain review:** whether an OM/size correction that yields negative
concentrations is scientifically acceptable — or signals a mis-specified /
over-extrapolated regression — is a domain question. The code-review point is
narrower: the guard conflates two very different causes under one message.

**NaN thread — parked (pick up later).** The separate investigation of how a
retained `nan` cell (Bank-B5, from a `NaN` string in the sheet) affects an
actual analysis run was **not completed**: it is blocked behind this
corrected-value negative guard (Cobalt/Mn/etc.), which must be cleared first.
Confirmed so far: `NaN` is accepted by `toDouble` as numeric, flagged "Ignored
NaN, Inf, or -Inf value" at import, and **retained visibly as `nan`** in the
Samples/Bank data table (not silently dropped). Its downstream effect on a
computed result remains the open question for a later session.

---

## GUI observations (usability + one functional bug)

### G1. "Include all elements" checkbox has INVERTED logic (functional bug)

**Observed:** with the **"Include all elements"** box **checked**, every element
in the "Include in analysis" column showed **"No"** (i.e., all *excluded*) —
the opposite of what the checkbox label states.

**Root cause (confirmed in `formelementinformation.cpp:28–36`):**

```cpp
if (checkBox->checkState() == Qt::Checked) {
    checkBox->setText("Include all elements");
    ...->IncludeExcludeAllElements(false);   // false = EXCLUDE all (per sourcesinkdata.h:1462)
} else {
    checkBox->setText("Exclude all elements");
    ...->IncludeExcludeAllElements(true);    // true = INCLUDE all
}
```

The argument is inverted relative to the label: **checked → relabels to
"Include all elements" but calls `IncludeExcludeAllElements(false)` → excludes
all.** (`sourcesinkdata.h:1462` documents the param as "if true, include all; if
false, exclude all", confirming the calls are backwards.)

**Severity: functional bug, not cosmetic.** A user who checks "Include all
elements" expecting all tracers active instead gets them all *excluded*; a
subsequent analysis then runs on no elements (or errors). Because this is the
tracer-selection control the entire analysis depends on, the inversion
undermines confidence in "which tracers did my run actually use." **Fix:** swap
the two `IncludeExcludeAllElements` arguments (and preferably also fix the label
pattern — see G2).

**[WESTENBROEK NOTE: this may be a lot of words from Kiro. I'd say, just ensure that the behavior of the checkboxes matches your intentions.... worth checking. Same applies to G2.]**

### G2. Label-flipping checkbox (usability anti-pattern) — and it caused G1

The same control **changes its label based on its state**: "Exclude all
elements" when unchecked, "Include all elements" when checked
(`formelementinformation.cpp:10,30,35`). A checkbox whose label flips is a known
anti-pattern: it becomes ambiguous whether the label describes the *current
state* or the *action of clicking*. Here it is not merely confusing — the
state-vs-transition ambiguity is precisely what produced the inverted logic in
G1. **Recommend** a fixed label ("Include all elements") whose checked/unchecked
state carries the meaning, which also removes the confusion that led to G1.

### G3. Multi-click element deselection (usability, low severity)

Toggling a single element's inclusion appears to require multiple clicks rather
than a single toggle. Low-severity friction; a one-click toggle on the "Include
in analysis" cell would be the expected interaction. (Factual usability note,
not a defect.)

> These are the GUI-layer observations; the numerical/correction/guard findings
> above are separate. G1 is the only functional bug of the three.

---

## Test 7 — Distribution Fitting crash (RESULT: reproducible crash, root-caused in source)

> **Scope note.** This finding was surfaced during *build/run verification*
> (Section 6 of `CODE_REVIEW_OBSERVATIONS.md`), not an exhaustive GUI test. The
> crash is used here as **reproducing evidence for a code-level defect** — a
> non-existent `append(series, name)` overload plus a throwing name-lookup — of
> the same family as the §12a unchecked-access findings. It is deliberately
> **not** a campaign to map every element/group combination.

### Reproduction (GUI)

1. Launch SedSat3; import `examples/SampleData.xlsx` (all tabs).
2. Tools → **Distribution Fitting**; select Constituent = **Barium**,
   Source/Target group = **Bank**; click **OK**.
3. Program pauses briefly, then exits. Console echo:
   `Tool executed QVariant(QString,"Distribution Fitting") : QVariant(QString,"DF")`.

**Reproduces regardless of OM/size correction state and regardless of the
"Use only selected samples" checkbox.** This independence is itself a result: it
rules out the negative-value / correction path as the trigger and localizes the
fault to the result-assembly + container API (i.e., it is *structural*, not
data-dependent).

### Debugger evidence (RelWithDebInfo build with PDBs)

Unhandled `std::out_of_range`. Symbolicated stack (top frames):

```
vcruntime140.dll                                   ; throw
SedSat3.exe!TimeSeriesSet<double>::operator[](const std::string& name)  Utilities/TimeSeriesSet.hpp:302
SedSat3.exe!Conductor::ExecuteDistributionFitting(...)                  src/conductor.cpp:1590
SedSat3.exe!Conductor::Execute(...)                                     src/conductor.cpp:119
SedSat3.exe!MainWindow::Execute(...)                                    mainwindow.cpp:1258
SedSat3.exe!GenericForm::onProceed()                                    genericform.cpp:197
```

`Locals` at the throw: the `TimeSeriesSet` being indexed had `size=2`, but its
series carried **empty names** (`name=""`) and `[0]` had **zero points**
(`{size=0}`); the lookup key was a non-empty name (`"Observed"`/`"Fitted"`) that
matched no series → `throw`.

### Root cause (confirmed in source)

Two combining defects, both code-level and generalizable:

**1. A `TimeSeriesSet::append(series, name)` overload that does not exist.**
The distribution-fitting assembly calls, e.g. in
`ConcentrationSet::CreateFittedDistribution` and `CreateCDFComparison`
(`src/concentrationset.cpp`):

```cpp
result.append(CreateDataCDF(), "Observed");   // (TimeSeries, string)
result.append(fitted_pdf,      "Fitted");      // (TimeSeries, string)
```

But `TimeSeriesSet` (`Utilities/TimeSeriesSet.h`) declares only:

```cpp
void append(const std::vector<T>& values);           // a row of values
void append(double t, const std::vector<T>& values);  // a row with time
void append(const std::string& name);                 // a NEW EMPTY named series
```

There is **no `append(const TimeSeries<T>&, const std::string&)`**. The
`(series, name)` calls therefore resolve to some other overload via implicit
conversions rather than the intended "store this series under this name," so the
stored series **never receive the names `"Observed"`/`"Fitted"`** — exactly the
empty-named series the debugger showed. (That this compiles at all is itself a
concern: a silent wrong-overload bind instead of a compile error. A `TimeSeries`
argument is being consumed by a `double`/`vector`-shaped overload.)

**2. `TimeSeriesSet::operator[](const std::string&)` throws unhandled on a
missing name.** (`Utilities/TimeSeriesSet.hpp:302`):

```cpp
throw std::out_of_range("TimeSeriesSet: Series name not found: " + name);
```

Combined with (1), the subsequent `fitted_normal["Observed"]` /
`fitted_lognormal["Fitted"]` lookups in `ExecuteDistributionFitting`
(`conductor.cpp:1588–1590`) are guaranteed to fail and terminate the app. A
`Contains(name)` method exists on the class but the call sites do not use it,
and the fitting code never validates that the fit produced the expected series
before indexing them.

### Assessment (code-review scope)

This is a genuine, reproducible **functional crash**, of the same class as the
§12a unchecked-access defects (`CBinary::operator[]` OOB; the missing-return
guard): a container accessor that **throws/deref-fails on an absent key with no
caller-side guard**, combined with a **produced-then-unvalidated** result
pattern. The proximate cause (throw on missing name), the intermediate state
(empty-named, partly-empty series), and the root cause (non-existent
`append(series,name)` overload) are all confirmed by debugger + source.

**Recommendations.**

1. **Add the intended overload** `TimeSeriesSet::append(const TimeSeries<T>&, const std::string&)` that stores the series *and* sets its name; audit all
   `append(series, name)` call sites to bind to it. (Consider `= delete`-ing or
   `static_assert`-guarding the accidental conversion so the wrong bind becomes a
   compile error.)
2. **Guard the name lookups** at the call sites (`Contains(name)` before
   `operator[]`, or an accessor returning an optional/pointer), and have
   `CreateFittedDistribution`/`CreateCDFComparison` return a well-formed,
   validated set (or a clear failure) rather than a silently mis-named one.
3. **Surface a user-facing message** for a fit that cannot be produced (e.g.
   lognormal on non-positive data — mirroring the existing `SelectBestDistribution()`
   fallback) instead of an unhandled throw.

### Incidental observations (not the crash)

- **"Perform Organic Matter and Size Correction first!" popup.** The guard at
  the top of `ExecuteDistributionFitting` checks whether OM/size **constituents
  are designated** (`Data()->OMandSizeConstituents()[0]/[1] == ""`), not whether
  a correction dialog was run. It only cleared after the LM correction dialog set
  those constituents — a wiring/UX wrinkle worth a note, unrelated to the crash.
- **License flag (feeds §13).** `Utilities/TimeSeriesSet.hpp` (and siblings in
  `Utilities/`) carry an **OpenHydroQual, GNU Affero GPL-3.0 (AGPL-3.0)** header
  — a *different* project and a **more restrictive** copyleft than any license in
  the §13 inventory. If these `Utilities` sources are AGPL, that is a distinct
  and stronger license-conflict data point than the GPL dependencies already
  flagged, and the `Utilities` submodule's stated Apache-2.0 (§13 table) may be
  inconsistent with these per-file headers. Recommend the release coordinator
  verify the actual license of the `Utilities` sources as shipped.
