# SedSAT3 — Response to the USGS code review (October 2026)

**Review:** Stephen Westenbroek, USGS software code review, 15 October 2026
(`CODE_REVIEW_SUMMARY_15OCT2026.md`, `TEST_RESULTS.md` in this folder), on
`code.usgs.gov/cphipps/SedimentSourceAssessmentTool3` at `ce7bc1d`.
**Response:** branch `review-fixes-oct2026`, starting from `bde1341`
(the development line, which already contains `ce7bc1d`).

| Commit | Content |
|---|---|
| `ac1dcd8` | Review notes, test projects and regression baseline recorded before any change |
| `41393e0` | Phase 1 — licensing, attribution and USGS metadata |
| `ac039a2` | Phase 2 — the confirmed code defects |
| `cdfa4ea` | Phase 3 — seeded, thread-independent random numbers |
| `68be7ca` | Phase 4 — screening of imported values |
| `2f8c463` | Phase 5 — tests, CI, optional GA, build cleanup, warnings |
| `538ed73` | This document; negative-value message distinguishes values made by the correction; the MCMC's periodic write of the samples file checks that the file opened |

Tests are in `tests/unit/tst_sedsat3.cpp` (Qt Test; named below as
`unit:<test>`) and `tests/regression/` (named `regression`). Both run in CI
(`.gitlab-ci.yml`, job `test:linux`) and locally with `ctest`.

## Item by item

### Institutional items

| Review item | Status | Commit | Test | Notes |
|---|---|---|---|---|
| Threshold question: is this a USGS product? | **Open — release coordinator** | `41393e0` | — | The repository now presents itself as USGS software: USGS header on every first-party file, USGS metadata, no links to personal repositories. Whether the code can be released by USGS, given its authorship, is for the release coordinator; nothing in the code decides it. |
| 1. AGPL / commercial-license headers in `Utilities` | **Fixed** | `41393e0` | — | The `Utilities` submodule (the author's personal GitHub library, shared with OpenHydroQual) is replaced by the 23 files SedSAT3 builds, copied into the repository from its commit `5db8d14`. The OpenHydroQual / AGPL-3.0 / "purchase a commercial license" headers on `TimeSeries*.{h,hpp}` are replaced by the SedSAT3/USGS header, which every first-party file now carries. The author (A. Massoudieh) made these copies for this purpose; OpenHydroQual's own copies are unchanged. A written statement from the author may be wanted for the release file. |
| License | **Placeholder** | `41393e0` | — | `LICENSE.md` reads `<<CC0 1.0 Universal / MIT — confirm with Cara>>`, with the USGS CC0 text ready in a comment. `code.json` and `CITATION.cff` carry the same placeholder. |
| 2. GPL dependency chain (GSL, QCustomPlot, Qt Charts) | **Documented — decision for release authority** | `41393e0` | — | Following the reviewer's note (SWB2's approach), `THIRD_PARTY_LICENSES.md` lists every dependency, how it is used and its license, and states that a binary package linking the GPL components is distributed under GPL-compatible terms. Nothing was re-engineered. |
| 3. Authorship / release authority | **Open — release coordinator** | `41393e0` | — | Author list is a placeholder in `CITATION.cff` and the README citation; the About dialog keeps Massoudieh, Gellis, Peterman-Phipps. |
| Citation | **Fixed (placeholders)** | `41393e0` | — | README citation in USGS software-release form; `CITATION.cff` added. DOI and author list are placeholders. |
| Metadata points at personal GitHub (`code.json`, README, `createdeb.sh`, submodules) | **Fixed** | `41393e0`, `2f8c463` | — | `code.json` and README rewritten with USGS GitLab placeholders; `createdeb.sh` builds and checksums only (no GitHub publishing); `Utilities` submodule removed; QXlsx submodule points at its upstream. `docs/CONTINUING-WORK.md` keeps a generic reference to a development mirror, as the reviewer allowed. |
| Windows installer from sedsat.org is a different lineage ("CMBSource", EnviroInformatics) | **Fixed in this repository; parallel distribution for coordinator** | `41393e0` | — | README no longer links sedsat.org. Installer project (`CMB_Source.aip`): Manufacturer "EnviroInformatics" → "U.S. Geological Survey" (this changes the default install folder to `Program Files\U.S. Geological Survey\SedSat3`), URLs → USGS placeholder. The portable package (below) is built from this repository. |
| README truncated / out of sync | **Fixed** | `41393e0` | — | Rewritten: features, manual (placeholder), portable build, build instructions and options, tests, disclaimer, license, citation. |
| No per-file license headers; no dependency (NOTICE) inventory | **Fixed** | `41393e0` | — | Header on 149 first-party files; `THIRD_PARTY_LICENSES.md`. |

### Code defects

| Review item | Status | Commit | Test | Notes |
|---|---|---|---|---|
| B1. Distribution Fitting crashes (`std::out_of_range`) | **Fixed** | `ac039a2` | `unit:distributionFitting_degenerateSets`, `unit:distributionFitting_conductor`, `regression` (DF steps) | The root cause differs from the review's: `append(series, name)` exists in this version. `CreateFittedDistribution` did `result["Observed"] = maxC/2`; because `TimeSeries(size_t)` was not `explicit`, the number became an unnamed empty series and the later `["Observed"]` lookup threw. Also fixed in the same path: the log-normal series was dropped by a duplicate-name check and `setname(2, …)` then went out of range; empty or constant data looped forever; the K-S statistic indexed a missing series. `TimeSeries(size_t)` is now `explicit`; the analysis checks group, constituent, sample count and positivity and reports a message; `MainWindow::Execute` catches any exception from any analysis and shows it. |
| G1/G2. "Include all elements" inverted; label flips | **Fixed** | `ac039a2`, `2f8c463` | `unit:includeAll_dataLevel`, `unit:includeAll_checkBox` | Fixed label "Include all elements"; checked = all included; the box follows per-element changes. `IncludeExcludeAllElements` has no other caller. The element table also read the text "No" as *include* (`QVariant("No").toBool()` is true); fixed. |
| G3. Several clicks to toggle one element | **Not changed** | — | — | Usability; the delegate opens a check box editor on double-click. Left for a GUI pass. |
| Missing `return` in `CheckNegativeElements` (both overloads) | **Already fixed** on `main` (`238dd69`) | — | `unit:negativeCheck_clean`, `unit:negativeCheck_negative` | Six other functions fell off their end (GCC `-Wreturn-type`); all fixed. `-Werror=return-type` (GCC/Clang) and `/we4715` (MSVC) are now set in every build file, so this cannot recur. |
| Negative-value guard skipped a source group | **Fixed (found during this work)** | `2f8c463` | `unit:negativeCheck_sourceAfterTarget` | `NegativeValueCheck` (and `GrandMean`, `LumpAllProfileSets`) stopped before the last group alphabetically, assuming it was the target. With target "Mixture" the source "Pasture" was never checked and the target was. Now every group but the target. |
| Negative-value message does not say the correction made the negative | **Improved** | Phase 6 | — (needs a project with an OM/size regression; not exercised by a test) | When the check runs on the corrected dataset, a value that is positive in the imported data is reported as "(after the organic matter and size correction; the imported values are positive)". Whether such a correction is acceptable is the domain review's question. |
| Null dereference in `CBinary::operator[]` | **Fixed** | `ac039a2` | `unit:binary_indexing` | Throws `std::out_of_range`. The same deliberate null dereference was in `CVector::operator[]` and `CVector_arma::operator[]`, used throughout the analyses; those now throw too, so an indexing mistake gives a message (via the GUI's exception handler) instead of a crash. |
| Writes to a closed `FILE*` in `src/GA/GA.cpp` | **Fixed** | `ac039a2` | — | `src/GA/GA.cpp` was a stale copy of the GA that no build compiled (it includes a `StringOP.h` that does not exist); removed. The GA that is compiled (`include/GA/GA.hpp`) now uses a `unique_ptr<FILE>` handle and skips writing when `fopen` fails; a `printf` with three `%le` and one argument and an unsynchronised counter in its OpenMP loop were fixed. Every other `fopen`/`fclose` in the code now checks for failure. |
| `log()` of a non-positive value in Box-Cox | **Fixed** | `ac039a2` | `unit:boxcox_*` | Data with a zero or negative value is left untransformed (as negatives already were; zeros gave −inf) and the analysis lists the skipped constituents. λ = 0 is the log case; for \|λ\| < 0.01 the transform uses `expm1` to keep precision; elsewhere the formula is unchanged. Outlier detection used its own copy of the formula, which shifted the scores of skipped constituents by 1/σ; it now uses the shared one. `CalculateBoxCoxLogLikelihood` (unused) took the log of a sum that was always negative; corrected. |
| Uninitialised variables | **Fixed** | `ac039a2` | CI `clang-tidy` (errors) | All 128 `cppcoreguidelines-init-variables` / `pro-type-member-init` findings. In-class initialisers for members; NaN for floating-point locals so a read-before-write is visible. Stepwise DFA could report an unassigned Wilks' λ; it now reports NaN. cppcheck `uninitvar` was not run locally (see Static analysis). |

### Reproducibility and random numbers

| Review item | Status | Commit | Test | Notes |
|---|---|---|---|---|
| MCMC not reproducible; no seed control or record | **Fixed** | `cdfa4ea` | `unit:mcmc_sameSeedSameResult`, `unit:mcmc_differentSeedDifferentResult`, `unit:random_*` | New "Random seed" field on CMB Bayesian, CMB Bayesian-Batch and Test CMB Bayesian (blank = drawn from the OS). The seed is shown in the results name ("MCMC results for 'X' (random seed N)"), stored with the project, in the CLI report, and on the first line of the samples file. Same seed and settings → byte-identical output. RNG: `std::mt19937_64` and `std::seed_seq`, with the uniform and normal values computed in `include/randomstream.h` rather than by `std::*_distribution`, whose algorithms differ between standard libraries, so a seed reproduces across GCC, Clang and MSVC. |
| Global `rand()` inside an OpenMP region | **Fixed** | `cdfa4ea` | `unit:mcmc_independentOfThreadCount` | Each chain has its own stream, `RandomStream(seed, chain)`, for the random start and every step, so the samples are identical with 1, 3 or 8 threads. Audit of every OpenMP region: the MCMC step loop also wrote a shared cached maximum through `predicted.SetRow` from every thread (now a named critical section); the GA loop's counter is atomic. Every other `rand()`, `srand()` and clock seed (the `Utilities` helpers, the GA, `TimeSeries`' GSL generator) now draws from a per-thread stream with a fixed seed. |
| Levenberg–Marquardt "exactly reproducible" | **Made genuinely reproducible** | `cdfa4ea` | `regression` | LM's starting point came from unseeded `rand()`: repeatable on one machine only while nothing reseeded it (the Windows MCMC did) and different between Linux and Windows. It now comes from a fixed-seed stream created for each solve. See the regression summary: for one target sample the result depends noticeably on the starting point. |
| No convergence diagnostics in the GUI | **Not changed — handoff** | — | — | The chains exist (`MCMC.h`), but R-hat / ESS / trace plots are not computed or shown. A feature for a later release; the size of the residual spread is for the domain review. Note for reviewers: "Number of samples" and burn-in count samples across *all* chains (1000 samples with 8 chains is 125 steps per chain). |

### Input validation

| Review item | Status | Commit | Test | Notes |
|---|---|---|---|---|
| NaN / non-numeric input (Test 4b) | **Fixed** | `68be7ca` | `unit:parser_*` | One parser, `include/numericparsing.h` (`parseNumericCell` and overloads), used for Excel import, project load, the element table and the CLI. Refused: empty, NaN/Inf/Infinity in any case, overflow, text, decimal commas. A refused value stops the import with sheet/row/column/value (Excel) or group/sample/constituent (project) — the treatment empty and non-numeric cells already had; SedSAT3 has no missing-data value. The "Ignored NaN, Inf, or -Inf value" message the reviewer saw came from Qt Charts while plotting: the NaN *was* stored. Project load used to turn a missing or NaN value (Qt saves NaN as null) into 0, and drop a failing sample silently. |

### Tests, CI, GA, cleanup

| Review item | Status | Commit | Test | Notes |
|---|---|---|---|---|
| No automated tests / CI | **Fixed** | `2f8c463` | — | 47 Qt Test checks; regression on two projects (Box-Cox, outliers, K-S, distribution fitting, ANOVA, correlation/covariance, bracketing, DFA, EDP, LM batch) with relative tolerance 1e-9; `.gitlab-ci.yml` builds (CMake, CMake with GA, qmake; manual Windows job), tests, clang-tidy, cppcheck, Pages. |
| GA advertised but not built | **Fixed** | `2f8c463` | CI `build:linux-ga` | The GA is kept but excluded by default (`CONFIG+=ga` / `-DENABLE_GA=ON`); both settings build. README and `code.json` no longer list it as a method; README says it is included but disabled, and why; the CLI and its manual leave out its commands. `MCMC.h` no longer includes `GA.h`. |
| Dead QXlsx submodule URL | **Resolved** | `2f8c463` | fresh `git clone --recursive` | The QXlsx URL (`j2doll/QXlsx`, which now redirects to `QtExcel/QXlsx`) works and its pinned commit is fetchable; it now points at `QtExcel/QXlsx`. The failing clone was most likely the `Utilities` submodule, which is gone. A fresh recursive clone of the branch builds. |
| Committed build artifacts | **Fixed** | `2f8c463` | — | Removed `wasm_bin/` and the stale `CMBSource.pro`; `.gitignore` covers build folders, IDE files and binaries. Linux no longer links a prebuilt `libQXlsx.a`; QXlsx is compiled from the submodule. |
| Spelling (`purturbation`, `FileBrowserPushButtom`, "samle") | **Fixed** | `2f8c463`, `cdfa4ea` | — | Also "Discription" (shown in the GUI as a label) and `sizeconsituent_`. The old MCMC setting names are still accepted, and the `purt_coeff_N` columns of the samples file are unchanged. "Pecentage eliminated" (an argument name scripts use) is unchanged. |
| Three build systems, no source of truth | **Improved** | `2f8c463` | CI | qmake projects share `sedsat3_core.pri`; CMake builds GUI, CLI and tests with the same source list; `SedSat3.vcxproj` kept for Visual Studio. README documents all. |
| Repo layout mid-transition | **Not changed** | — | — | Moving the root sources under `src/` is a large mechanical change best done separately. |

### Further defects found and fixed during this work

| Defect | Commit | Effect |
|---|---|---|
| ANOVA ignored "Use only selected samples/elements" | `2f8c463` | ANOVA results change (below). |
| `CMBTimeSeries::operator=`, `CVector_arma::operator*=` called themselves | `2f8c463` | Stack overflow if used. |
| `CMatrix_arma(const CVector_arma&)` built a temporary, then wrote past the end of an empty matrix | `2f8c463` | Memory corruption if used. |
| `CMatrix_arma_sp::get` returned a reference to a local | `2f8c463` | Writes to sparse matrices were lost (class unused by analyses). |
| `RangeSet` copy constructor copied nothing; result-type copy constructors dropped their `Interface` base | `2f8c463` | Copies lost data / notes and axis labels. |
| `TimeSeries::addPoint` did not compare the first two intervals | `2f8c463` | Uneven series could be treated as evenly spaced. |
| `CMCMC::model` repeated its work once per parameter | `2f8c463` | Wasted time only. |
| `gamma()` clashed with `::gamma` under Clang | `ac039a2` | Clang build failure. |

## Follow-up to the supplementary documents (October 16)

Steve's second email added `CODE_REVIEW_OBSERVATIONS.md`, `linter_output.txt`,
`REVIEWER_TEST_PLAN.md`, `RABBIT_HOLE_rand_and_threads.md`,
`BUILDING_ON_WINDOWS_WITH_PIXI.md` and the USGS release checklist (all in this
folder). Every concrete finding was checked against the code; the item-by-item
answers are in the response letter,
[`response-letter/SedSAT3_code_review_response.pdf`](response-letter/SedSAT3_code_review_response.pdf)
(Sections 7–11). Fixed in the follow-up commit:

| Defect | Where | Test |
|---|---|---|
| `Interface` copy constructor / assignment dropped notes and highlight limits (outlier and bracketing flags lost on copy); `CMBVector::operator=` skipped its base | `src/interface.cpp`, `cmbvector.cpp` | `unit:interface_copiesKeepNotes` |
| Null dereference: isotope observed data read the base element from an empty sample name | `src/sourcesinkdata.cpp` (`ObservedDataforSelectedSample_Isotope`) | — |
| t-test degrees of freedom (`size_t`) wrapped around with < 2 samples | `src/sourcesinkdata.cpp` (`t_TestPValue`) | — |
| Uniform/Dirichlet distribution curve made of NaN points; one-point curve divided by zero | `src/cmbdistribution.cpp` | `unit:distribution_uniformCurveIsFinite` |
| Results table compared the time, not the value, with the upper highlight limit | `cmbtimeseries.cpp` | — |
| MCMC initialisation step size used −low − high | `include/MCMC/MCMC.hpp` | — |
| Error Analysis drew from a shared stream (result depended on earlier runs); now its own seed field, recorded in the results name | `src/conductor.cpp`, `src/sourcesinkdata.cpp`, forms | `unit:errorAnalysis_sameSeedSameResult` |
| `GA.hpp` not self-contained (`Q_version` guards, a macro defined nowhere); `CGA::operator=` signature | `include/GA/GA.hpp`, `GA.h` | CI `build:linux-ga` |
| Hard-coded `#define ARMA_USE_SUPERLU` broke builds without SuperLU (Windows/conda-forge) | `Utilities/Matrix_arma*.cpp` | — |
| MSVC C4005/C4828: unguarded `USE_QCHARTS`, `NOMINMAX`; no `/utf-8`; `#include < time.h >` | `plotwindow.h`, `Utilities/Utilities.cpp`, `CMakeLists.txt` | — |
| Visual Studio project compiled QCustomPlot from outside the repository | `SedSat3.vcxproj` | — |
| Unused `FilePushButton` class, empty `GA_impl` files; stale `createdmg.sh` version | removed / updated | — |

Also added: tests for the reviewer test plan's Test 2 (contributions sum to one and
are non-negative, both parameterisations) and Test 4d (single-sample source),
`concurrency-mt-unsafe` as an error in `.clang-tidy`, and a codespell CI job
(reported, not enforced). Regression comparison: no differences.

Still open from the supplementary documents: a corrected `pixi.toml` (to be
tested on Windows), the Visual Studio project's prebuilt QXlsx library, the GA's
identical `apply_to_all` branches (intent to confirm), `performance-*`
clang-tidy style items, MSVC `/W4` and the first cppcheck report, and the
G3 single-click toggle.

## Domain review (L. Gorman Sanisaca, October 21)

The domain review tested release v1.1.6 (December 2025). Each crash was
reproduced against this branch with the example data; the response is
`docs/review/response-domain-review/`.

| Finding | Commit | Test |
|---|---|---|
| Fingerprinting, batch, Bayesian, Error Analysis and Source Verification crashed when an isotope's base element was excluded; now a message | `619f0a6` | `unit:fingerprinting_isotopeBaseElementExcluded` |
| Elements' discriminant power (multi-way) read group arguments its form lacks | `619f0a6` | `unit:multiwayDiscriminantPower_runs` |
| Correlation matrix with the correction and "Use only selected elements" filtered out TOC before correcting | `619f0a6` | `unit:correlationMatrix_correctedWithSelectedElements` |
| OM/size regression aborted (GSL) for a group with no more samples than coefficients | `619f0a6` | same test |
| D50/TOC tested and selected as tracers by ANOVA, DFA, discriminant power, auto-select and bracketing; "Modify the included elements" excluded them | `619f0a6` | `regression` (baseline updated, below) |
| Error Analysis histogram used only the first realization | `d0acec7` | — |
| Log-normal K-S test on non-positive data drew nothing; now refused | `839772d` | — |
| Charts: palette, axis titles and ranges, log axes below 1e-12, legends, DFA pair start; tools menu order and groups; Bayesian defaults 100,000/20,000; step 4 shows scores and eigenvectors | `619f0a6`, `d0acec7` | charts rendered offscreen and inspected |

**Baseline change** (`properties.cmb` only; `Isotope_test.cmb` gives D50 and
TOC the element role and is unchanged): D50 and TOC leave the ANOVA (steps
25–26), bracketing (29) and EDP (31) results, with the other values unchanged;
the two-way DFA (30) changes because they no longer enter the discriminant
function.

## Regression comparison

The scripts in `tests/regression/` were run with the code before the fixes
(`bde1341`, stored in `tests/baseline/bde1341/`) and after
(`tests/baseline/`). Every difference, on both projects:

| Analysis | Change | Explained by |
|---|---|---|
| Distribution fitting (8 steps) | failed ("Series name not found: Observed") → succeeds | B1 |
| Outlier analysis: C13 (all sources), N15 (Forest) | every z-score shifts by a constant 1/σ (e.g. −0.273 → 0.542) | Box-Cox: skipped constituents were scored from x − 1 |
| Box-Cox, Outlier | new warnings listing the constituents Box-Cox skips | Box-Cox |
| Levenberg–Marquardt batch | most contributions change by < 1e-5 (median 2e-6); `Isotope_test` max 0.0009; `properties` target 15 by up to 0.058 (Crop 0.544 → 0.486, Pasture 0.000 → 0.057) | LM start from a seeded stream instead of `rand()`. **For the domain review:** the LM solution for some samples depends on the starting point. |
| ANOVA, default options | p-values change, some across 0.05 (e.g. 0.0427 → 0.0143, 1.1e-6 → 1.5e-8) | ANOVA now uses all samples when "Use only selected samples" is false, as the form says, and (Isotope_test) includes the Pasture group it used to drop |
| ANOVA, selected samples only | changes on `Isotope_test` only | the dropped Pasture group |

Unchanged on both projects: Box-Cox λ, K-S statistics, correlation and
covariance matrices, bracketing, DFA, EDP. With the fixes, the two projects
(the same source samples, targets named "Mixture" and "Target") give the
same ANOVA, which they did not before.

**On Windows** (MSVC, conda-forge OpenBLAS; `pixi run test`), every analysis
matches the Linux baseline within the 1e-9 tolerance except the
Levenberg–Marquardt batch: 30 values on `isotope_test` and 37 on
`properties` differ, by at most 7e-8 (median relative 8e-9; the largest
relative difference, 3.6e-5, is on a contribution of 4e-5). This is round-off
in an iterative fit, so the baseline was not regenerated on Windows. The
regression test needs `bash`; on Windows, configure with
`-DBASH_EXECUTABLE="C:/Program Files/Git/bin/bash.exe"`.

## Compiler warnings and static analysis

GUI build, GCC 13.3, Qt 6.8.3, unique first-party warnings
(`tools/count-warnings.py`):

| Flags | Before | After |
|---|---|---|
| qmake default (`-Wall -Wextra`) | 244 | 100 |
| `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` | 618 | 448 |

Fixed: every `-Wreturn-type`, `-Winfinite-recursion`, `-Wreturn-local-addr`,
`-Wformat`, `-Wmaybe-uninitialized`, `-Wmisleading-indentation`,
`-Wreorder`, `-Wtype-limits`, unused variable/parameter/function, invalid
escape and copy-constructor warning. Remaining: `-Wconversion` 293,
`-Wsign-compare` 100, `-Wshadow` 35 (mostly constructor parameters named after
members), `-Wfloat-conversion` 20 — implicit integer/floating conversions in
the numerical code, which should be changed one at a time with the regression
tests rather than in bulk.

**MSVC `/W4`** (`pixi run warnings`; Windows 11, Visual Studio 2022, MSVC
v143), unique first-party warnings:

| Warning | Before | After |
|---|---|---|
| C4834 discarded `[[nodiscard]]` (unchecked `QFile::open`) | 5 | 0 |
| C4267 `size_t` narrowing | 202 | 202 |
| C4018 signed/unsigned comparison | 28 | 28 |
| C4244 conversion, possible loss of data | 18 | 18 |
| C4458 declaration hides class member | 13 | 13 |
| C4996 deprecated function | 12 | 12 |
| C4456 declaration hides previous local | 7 | 7 |
| C5043 | 1 | 1 |
| **Total** | **286** | **281** |

The five C4834 were unchecked `QFile::open` calls: project load, `loadJson`
and `saveJson` in `mainwindow.cpp`, and the result exports in
`resultswindow.cpp` and `resulttableviewer.cpp`. The default (non-`/W4`) build
also had a C4805 (`int == true`) in `src/GA/Binary.cpp`, fixed without
changing behaviour. The default build is now warning-free. The remaining
conversions are the MSVC counterpart of the GCC `-Wconversion` and
`-Wsign-compare` findings and will be handled the same way.

**clang-tidy** (LLVM 20, `.clang-tidy`): 128 initialisation findings and the
real defects above fixed. 25 remain, none of the error class: 7
`clang-analyzer-security.FloatLoopCounter` (grid loops over x and λ, intended),
4 `NonNullParamChecker` that the analyser cannot see are guarded, 2
`NewDeleteLeaks` on Qt-parented labels (false positives), 3
`exception-escape` from Armadillo destructors, and 9 style findings (unused
locals, empty `catch`, `_Data` names, repeated branches).

On Windows (clang-tidy from conda-forge, `pixi run lint`): no errors, 10
warnings: 3 float loop counters, each reported by two checks; 2 static
initialisers that could throw (`script_data` in `cli/scriptrunner.cpp`, `pi`
in `src/cmbdistribution.cpp`); and the 2 `NewDeleteLeaks` false positives.

**cppcheck** (`pixi run cppcheck`, on Windows): 158 findings, 150 after the
fixes. Real defects fixed:

- `CMatrix_arma::get(int)` wrote into an empty `std::vector` (out of bounds)
  and looped over `numrows` instead of a row's `numcols`. It has no callers.
- `TimeSeries` interpolation divided by `p2.t - p1.t` before testing it for
  zero.
- `SourceSinkData::MCMC` left `mcmc->Model` pointing at a local
  `SourceSinkData` after returning. Nothing read it afterwards; it is now
  reset to `nullptr`.
- `CGA::Model` was not initialised by the constructors.

Three `zerodiv` reports in `Utilities/TimeSeries.hpp` are on divisions already
guarded by a comparison with `T{}`, and carry inline suppressions. Remaining:
60 `postfixOperator`, 46 `passedByValue`, 15 `returnByReference`, 14
`useInitializationList`, 11 `operatorEqVarError`, 3 `c_str()` performance
findings, and 1 `invalidFunctionArg` (`log(0)` if every value is equal, in
`ConcentrationSet`'s Box-Cox log-likelihood, which has no callers). cppcheck
also runs in CI (`cppcheck` job, reported but not yet enforced).

## Windows portable package

`docs/PACKAGE-Windows-Portable.md` is the full procedure, with verification.
In short, on the Windows machine (Visual Studio 2022, Qt 6 MSVC kit, the
`..\armadillo` and `..\vcpkg` layout `SedSat3.vcxproj` expects):

1. Update the clone: `git fetch`, `git checkout review-fixes-oct2026` (or
   `main` once merged), `git submodule update --init --recursive`.
2. Build Release x64:
   `MSBuild.exe SedSat3.vcxproj -p:Configuration=Release -p:Platform=x64 -m`.
   Check with `dumpbin /dependents x64\Release\SedSat3.exe` that it imports
   `MSVCP140.dll`, not `MSVCP140D.dll`.
3. Stage `C:\portable\SedSAT3\bin\bin\SedSat3.exe` (two `bin` levels: the
   program finds `..\..\resources`).
4. `windeployqt.exe --release --no-translations C:\portable\SedSAT3\bin\bin\SedSat3.exe`
   from the same Qt kit (not `--compiler-runtime`).
5. Copy the MSVC runtime and `vcomp140.dll` (OpenMP) app-local, and `gsl.dll`,
   `gslcblas.dll`, `blas_win64_MT.dll`, `lapack_win64_MT.dll`.
6. Copy `resources\tools.json`, `resources\forms_structures.json`,
   `resources\Icons\` to `C:\portable\SedSAT3\resources\`, plus the README and
   sample data.
7. Verify (dependency walk, restricted-token launch), then zip to
   `SedSAT3-<version>-windows-x64-portable.zip` and write its SHA-256.

**Done on Windows, version 1.1.8**: `SedSAT3-1.1.8-windows-x64-portable.zip`,
42 MB, SHA-256
`a934d3c97a5e8735111bd180f251bbcce3ea46c3983ca0501b9aa0be2fa2c060`. It was
built from the pixi build (`pixi run build`), not the vcxproj, so steps 2–5
differ:

- Qt was deployed with the environment's `windeployqt6 --qtpaths
  .pixi/envs/default/Library/bin/qtpaths6.exe --release --no-translations
  --no-compiler-runtime`, run on both `SedSat3.exe` and `sedsat3-cli.exe`.
- The numerical libraries the programs import are `openblas.dll`,
  `gsl-28.dll` and `libcblas.dll` (for GSL). `libblas.dll`, `liblapack.dll`
  and `libwinpthread-1.dll` from `deploy-win` are not needed.
- conda-forge's Qt needs 18 more DLLs that `windeployqt` does not copy (ICU,
  zlib, zstd, brotli, freetype, libpng, pcre2, double-conversion, OpenSSL's
  libcrypto for the TLS plugin, and the image-format libraries). They were
  found by walking the import tables against the package and `System32` only,
  repeating until nothing was missing (three rounds).
- `pixi.toml` now requests the OpenBLAS builds of `libblas`, `libcblas` and
  `liblapack`. On win-64 the MKL builds were chosen, and their `libcblas`
  forwards every call to `mkl_rt`, which would have added about 540 MB of MKL
  DLLs.

Verified: the dependency walk is complete. Under a restricted token, the GUI
opened with no UAC prompt from `%LOCALAPPDATA%`, from a path with spaces and
from a `subst` drive, with all 118 modules loaded from the package or
`System32`. The CLI ran the bundled example, including the Bayesian batch,
from the path with spaces. The example sets `"Random seed": "1234"`, so the
README lists exact expected results; two runs gave identical credible
intervals. 1.1.7 was the package built before the code-review fixes.

## Placeholders still to fill

| Placeholder | Where |
|---|---|
| `<<CC0 1.0 Universal / MIT — confirm with Cara>>` | `LICENSE.md`, `code.json`, `CITATION.cff` |
| `<<10.5066/XXXXXXX>>` (DOI) | `CITATION.cff`, `code.json`, `README.md` |
| `<<Massoudieh, A.; Peterman-Phipps, C.; Gellis, A.C.; … confirm full list>>` | `CITATION.cff`, `README.md` (About dialog lists the three names) |
| `<<https://code.usgs.gov/...final group path...>>` | `README.md`, `code.json`, `CITATION.cff`, `CMB_Source.aip`, `docs/BUILD-Windows-MinGW.md`, `docs/CONTINUING-WORK.md` |
| `<<USGS manual URL/DOI>>` | `README.md` |
| `<<version>>`, `<<YYYY-MM-DD>>`, `<<year>>` | `code.json`, `CITATION.cff`, `README.md` |
| `<<contact name>>`, `<<contact email>>` | `code.json` |
| `<<confirm: no third-party icon sets are included>>` | `THIRD_PARTY_LICENSES.md` |
| Approved-software disclaimer | `DISCLAIMER.md` has the provisional text; switch at release |
