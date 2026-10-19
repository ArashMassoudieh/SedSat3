# SedSAT3 — Code Review Observations

**[WESTENBROEK NOTES: Kiro kind of goes nuts with the writing. I really am not a fan. It makes pronouncements that may or may not be accurate. At the kernel, however, of each flagged item, lies a real issue that we stumbled over. I would suggest viewing these at a high level. Do some minimal investigation for each, and move on if it turns out to be overblown. An example is the treatise it wrote on the "Threshold Question". What a lot of.... words. The short version is: make the decision to either keep development and documentation as is (own domain: sedsat.org) or commit to getting this stuff into code.usgs.gov. And either commit to putting code into the USGS public-domain world or keep it as  part of OpenHydroQual.  Or do a more deliberate job of defining the bits that stay in OpenHydroQual and the bits that belong as USGS code subject to FSP.]**

**Reviewer notes (technical code review)**
**Date:** 2026-09-14
**Repo state reviewed:** superproject commit `ce7bc1d` on `main`
**Software:** SedSAT3 v1.1.6 — C++17 / Qt6 desktop application for sediment
source fingerprinting and apportionment.

> **Authorship.** These review comments were produced primarily by **Kiro** (an
> AI coding assistant), working interactively with and under the direction of
> **Stephen Westenbroek** (USGS). The investigative work — reading the source,
> running the build and static-analysis tooling, operating the GUI, tracing the
> crash in a debugger, and reading the license headers — was carried out by the
> assistant; the scoping, judgment calls, direction, and verification of the
> conclusions were a collaboration. Attribution is given plainly so readers know
> how the material was generated.

> **Provenance and reliability caveat (please read).** Because much of this
> review was AI-generated, treat it as a well-evidenced *starting point*, not an
> infallible audit. The findings are tiered by how firmly they can be stated,
> and that tiering is meaningful:
>
> - **High-confidence** items were verified by direct, reproducible evidence —
>   a debugger stack trace, a cited source line read in the tree, a tool run
>   (cppcheck/clang-tidy), an empirical GUI result, or a license header quoted
>   verbatim. These are reliable; where a specific line or file is cited, the
>   authors can confirm it directly.
> - **Lower-confidence** items — interpretation of still-evolving USGS policy,
>   single-tool static-analysis flags not yet runtime-confirmed, and anything
>   explicitly marked "not verified," "lower bound," or "worth your
>   confirmation" — should be independently checked before being relied on.
> - **Known limitations.** An AI-assisted review of this size can still contain
>   residual inconsistencies, stale cross-references, or mis-stated details
>   despite several consistency passes. Line numbers and quoted snippets reflect
>   the reviewed commit (`ce7bc1d`) and may drift as the code changes. Nothing
>   here is a legal, licensing, or scientific-validity determination — those are
>   explicitly routed to the release authority, the Solicitor, and the domain
>   reviewer. **When in doubt, trust the cited primary evidence (the code, the
>   tool output, the license text) over the surrounding prose.**

---

## Threshold question (read first): is this reimplementation a USGS software product?

> **This is the antecedent question, and it is for the release authority — not a
> code-review determination.** It is placed first because it frames everything
> below: if it is unresolved, the technical findings (§1–§16), however real and
> fixable, are secondary. The detailed authorship/authority discussion is in §15;
> this is the summary a reader should meet first.

**Lineage (verifiable, and it helps rather than accuses).** An original
**Sed_SAT** tool — written in **R with a Microsoft Access interface** — was
properly released by USGS: hosted under the organizational `water/` group on
code.usgs.gov, IPDS record **IP-079331**, published as **OFR 2017-1062**
(Gorman Sanisaca, Gellis, and Lorenz, 2017), under the USGS Software User Rights
Notice. That tool's USGS provenance is not in question.

**What is under review is a different codebase.** The software reviewed here is
**new, independently-authored C++17 / Qt6 code** that reimplements the *scientific
methodology* of that original tool — not a port of its code. This can be stated
unambiguously: the original is R scripts plus an Access database; the reviewed
artifact is C++/Qt source with its own build system. They share scientific
heritage and purpose (and the "Sed_SAT" name), not source code. *(The shared
heritage is of method and purpose; this review did not attempt — and does not
assert — any line-level derivation in either direction.)*

**Why that matters.** Because the reviewed C++ code is new, independent
authorship, its copyright and license status rests on *its own* authorship
facts; it does **not** automatically inherit the original R tool's public-domain
USGS status. And several verified observations about the reviewed codebase point
outside USGS:

- the first-party `Utilities` core carries **AGPL-3.0 + commercial** notices
  asserting copyright by **Arash Massoudieh / EnviroInformatics, LLC** (§13a);
- the publicly distributed Windows installer is branded **"CMBSource" by
  "EnviroInformatics,"** at a version and framework generation that do not match
  the reviewed source (§14a);
- the project's canonical identity — repository links, user manual, issue
  tracker, and a public download site (sedsat.org) — resolves to the author's
  **personal namespace**, not code.usgs.gov (§14, §14a).

**What this does and does not mean.** It does **not** mean the reimplementation
cannot be a USGS release — a non-federal author's work can be released by USGS
under the appropriate instrument (cooperative agreement, contract, CRADA, or an
explicit copyright/license grant), and the software is presented as developed in
collaboration with the USGS. The narrow, honest point is that **the authorizing
basis is not visible in the reviewed materials, and the public-facing evidence
points the other way.** Because the license and authorship questions (§13, §13a,
§15) are antecedent to the technical findings, they should be settled by the
release coordinator — and, as applicable, the Science Publishing Network and the
DOI Office of the Solicitor — before further effort is spent on the technical
items.

**[WESTENBROEK NOTES: I am a bit leery of Kiro's broad pronouncements, but the observations are true (AGPL license + CMBSource with copyright for MSI package); although I am uncomfortable with Kiro's wording, the facts would seem to support that we cannot call it a USGS software release and then distribute a package with private copyright all over it...]**

---

## 1. What the software is

- **Language / stack:** C++17, Qt6 (Core, Gui, Widgets, PrintSupport, Charts).
  GUI-driven desktop application; no CLI entry point.
- **Scientific scope:** sediment source apportionment via three approaches —
  deterministic maximum likelihood (Levenberg–Marquardt), genetic algorithms,
  and Bayesian MCMC — plus preprocessing/statistics (outlier detection, size/OM
  corrections, DFA, ANOVA, tracer selection, distribution fitting, etc.).
- **Core computational code:** `src/` and `include/` —
  `sourcesinkdata`, `conductor` (command dispatcher), `GA/`, `MCMC/`,
  `elemental_profile(_set)`, `concentrationset`, `parameter`, `cmbdistribution`.
- **Dependencies:** Qt6, Armadillo, GSL, OpenMP, QXlsx (submodule),
  qcustomplot6 (vendored in `thirdparty/`), and a `Utilities` submodule.

---

## 2. Repository state and hygiene

- **Clean tree, short history.** `main` is clean; history is compact and looks
  squashed/migrated ("Initial commit", "USGS pre-reqs", "Hygiene"). A
  `usgs/main` merge and an `import-sedsat-source` branch indicate a recent
  migration into the USGS-track repository. **[SMW note: this would appear to reflect a recent migration of the code from a personal repo into a code.usgs.repo; developmnet history is not available via git mechanisms in this repo...]**
- **Three parallel build systems:** CMake (`CMakeLists.txt`, the maintained
  one), qmake (`SedSat3.pro`, plus a legacy `CMBSource.pro`), and MSVC
  (`SedSat3.vcxproj`). No single documented source of truth. The legacy
  `CMBSource.pro` contains a hardcoded personal path
  (`/home/arash/Projects/QXlsx/...`) — cruft, not a secret.
- **Un-run hygiene script.** `apply-hygiene-cleanup.sh` exists but appears not
  to have been executed: `CMB_Source.aip` (Advanced Installer project, ~37 KB)
  is still tracked, and a stray lowercase `gitignore` file sits alongside **[SMW note: this appears to be part of a Windows build process... not included / working as a pipeline in this repo]**
  `.gitignore`.
- **Secrets / PII:** none found. A scan of source/config for credentials,
  tokens, private keys, internal hostnames, and RFC1918 IPs returned nothing.
  The only "password" hit is boilerplate text inside the vendored GPL license.

---

## 3. Fate of the git submodules — action required

This is the most consequential finding for reproducibility.

### 3a. QXlsx submodule points at a dead URL (build blocker, all platforms)

`.gitmodules` references `https://github.com/j2doll/QXlsx.git`, which now
returns **"repository not found."** A fresh `git submodule update --init`
therefore fails for everyone, directly contradicting `README.md`'s build
instructions. The project's pinned commit (`700bda6`) still exists at the live
upstream, `https://github.com/QtExcel/QXlsx.git` (tag context
`v1.5.0-17-g700bda6`).

**[WESTENBROEK note: keep a list of the external modules this code is linked to and note the licensing of each. QXlsx falls under a MIT license. Should have a high-level THIRD_PARTY_LICENSES.md to note all of this...]**

### 3b. Utilities submodule — dependency on an author-maintained repo

`Utilities` is pulled from `https://github.com/ArashMassoudieh/Utilities.git`
(the author's personal namespace), pinned at `5db8d14`. It is a general-purpose
math/utility library shared across the author's projects, not specific to
SedSAT3. Two review implications:

- **Single point of failure / longevity:** a USGS-released product depends on a personal
  GitHub repo outside USGS control. Consider vendoring the pinned Utilities
  sources into the release, or mirroring the dependency under a USGS namespace,
  so the release remains buildable independent of that account.
- **The SuperLU link failure was a pixi/conda-forge issue, not an intrinsic
  Windows one.** `Utilities` hard-codes `#define ARMA_USE_SUPERLU 1`
  (`Matrix_arma.cpp`, `Matrix_arma_sp.cpp`), and conda-forge has no `superlu`
  package for `win-64` — so the pixi/MSVC build path this review constructed
  failed to link until SuperLU was guarded off on Windows (see the Windows build
  guide, Pitfall 4). This is specific to that toolchain, not a statement that
  Windows cannot build the code. Because the define lives in `Utilities`, any
  permanent guard belongs upstream there. Note also that the reviewed repo ships
  **no Windows build path of its own** — the pixi/MSVC path was improvised for
  this review.

### 3c. qcustomplot6 is vendored, not a submodule

`thirdparty/qcustomplot6/` is committed directly (source + GPL). That is fine
for reproducibility, but the version/provenance is undocumented — worth a note
in the docs recording the upstream version.

---

## 4. Documentation

- **In-code documentation is a genuine strength.** 798 Doxygen `@brief` tags
  across 20 headers, with a `Doxyfile` and a GitHub Action (`docs.yml`) that
  auto-publishes Doxygen HTML to GitHub Pages. Core classes (e.g., `Conductor`)
  carry substantive class- and method-level documentation.
- **README** covers purpose, features, install (Windows installer / Linux .deb
  / build-from-source), a quickstart, and a description of the example dataset.
- **The README's opening paragraph is truncated mid-word (verified).** The very
  first descriptive sentence ends: *"…Bayesian inference through Markov Chain
  Monte Ca…"* — cut off at "Monte Ca" (should be "Monte Carlo (MCMC)"), trailing
  into an ellipsis. This is in the repo's `README.md` itself (not an artifact of
  how it was quoted). The *complete* version of this same sentence appears on the
  author's website (sedsat.org), so the repo README looks like a partial/stale
  copy. Trivially fixable, but it is the first thing a reader sees in a would-be
  USGS release.
- **Against SM 502.11 minimums** (purpose / methods / execution / dependencies):
  largely met for the intended platforms. Gaps:
  - No **Windows** build-from-source instructions (this review adds one; note
    the author *does* have a working Windows recipe — VS2022 + vcpkg — but it is
    published only on sedsat.org, not in the repo; see §14a).
  - Dependency versions are unpinned and the multi-build-system situation is
    undocumented.
  - `docs.yml` publishes generated Doxygen; a `docs/` folder of generated HTML
    was previously tracked (the hygiene script targets it) — confirm generated
    output is not committed.

### 4a. Documentation-provenance pattern (observation — several signals point one way)

Taken together, a number of **individually minor, individually verified** facts
suggest the repository's top-level documentation and metadata were assembled by
adapting pre-existing material (the predecessor "CMBSource" project and the
author's website) into a USGS-style presentation, rather than authored fresh for
this release — and that the pieces have drifted out of sync. The signals:

- The README's opening paragraph is truncated mid-word (above), while the
  website carries the complete text — consistent with a partial copy.
- The README advertises a **Windows installer** whose shipped artifact is a
  **different product and version** (CMBSource v0.0.20, Qt 5) than the repo
  (SedSAT3 v1.1.6, Qt 6) — see §14a.
- The README/`code.json` advertise a **genetic algorithm** that is not built or
  reachable in the current source (Test 6, §12d).
- `code.json`, README, `createdeb.sh`, and the submodules point at the author's
  **personal GitHub**, not code.usgs.gov (§14); the repo still carries
  predecessor-era "CMB" artifacts (`CMBSource.pro`, `CMB_Source.aip`,
  `CMBSource_en_US.ts`; §9).

**Framing (important).** This is an **observation about documentation/metadata
consistency**, not a judgment of intent — the review cannot and does not assert
why the material is in this state. The top-level docs/metadata are **internally inconsistent and out of
sync with both the shipped artifact and the current source**, in a way that
reads as inherited-and-adapted rather than authored-for-this-release. For a USGS
release that matters because README/`code.json` are the public-facing,
machine-read descriptors; they should describe *this* software, at *this*
version, from *this* repository. Reconciling them is low-effort but should be
done deliberately rather than by further copy-paste.

**[WESTENBROEK notes: Ugh, Kiro. Correct, but obnoxious.]**

---

## 5. Testing and reproducibility

- **No automated test suite.** The only test-named file, `testmcmc.cpp`, is a
  non-functional stub: `GetObjectiveFunctionValue()` returns `0`,
  `GetPredictedValues()` returns an empty vector. There are no unit/functional
  tests and no CI that builds or exercises the code (the sole GitHub Action
  only builds documentation).
- **Reproducible example:** `examples/SampleData.xlsx` plus a documented
  quickstart exists, but there is no scripted/automated workflow that runs the
  sample data and checks a known result. The USGS super-checklist calls for
  "unit or functional tests and/or executable examples that verify core
  functionality"; the sample dataset partially satisfies the "executable
  example" spirit but is manual and unverified.
- **Recommendation:** add at least one end-to-end regression check (run the
  sample dataset through one apportionment method and compare against a stored
  expected result within tolerance). This also directly supports the domain
  review.

---

## 6. Build verification result

The application was successfully built and launched on Windows (a non-target
platform) using pixi + MSVC. All 101 translation units compiled; the executable
links and the GUI launches. Full procedure, pitfalls, and evidence are in
`REVIEW_COMMENTS/BUILDING_ON_WINDOWS_WITH_PIXI.md`. Compiler warnings observed
(C4828 illegal UTF-8 bytes, C4834 discarded `[[nodiscard]]` returns, C4005
macro redefinition) are documented there and are worth addressing but are not
errors.

Note: only compilation/link/launch were verified. Runtime scientific
correctness was not exercised and belongs to the domain review.

---

## 7. USGS release-checklist alignment (quick status)

**[WESTENBROEK notes: this was long and tedious and sometimes only partly right. I deleted it entirely...]**

**[updated note. rubbish. largely noise.]**

---

## 8. Suggested priorities for the developers

> *Note (2026-10-15):* this list was reordered to reflect findings added after
> the original draft — the Distribution Fitting crash (§Test 7), the first-party
> AGPL finding (§13a), and the inverted-checkbox bug (G1). It now mirrors the
> priority structure in `REVIEW_SUMMARY.md` and
> `CODE_REVIEW_SUMMARY_FOR_AUTHORS.md`.

**Release-authority blockers (institutional — resolve first; not code fixes).**

1. **First-party AGPL / commercially-licensed core (§13a) — most consequential.**
   The `Utilities` core carries AGPL-3.0 + commercial "OpenHydroQual" headers
   asserting private copyright. Sharper than the GPL-library issue below because
   it is first-party core code, not a swappable dependency, and it collides with
   the CC0 premise. Resolve with the release authority (and likely the Solicitor).
2. **License conflict in the dependencies (§13).** GPL-3.0 is outside the
   USGS-preferred set (CC0/MIT/Unlicense), and three GPL deps — **GSL,
   qcustomplot6, and Qt Charts** — block a simple relicense. Replace them
   (plotting → QWT or self-drawn on LGPL Qt; GSL's one call → Armadillo/Eigen),
   or seek a documented exception. *(Qt core is LGPLv3 — not a blocker; the GUI
   need not be removed.)*
3. **Authorship / release authority (§15).** Establish whether SedSAT3 is a USGS
   product and who holds copyright — antecedent to the license decision.

**Concrete code defects (bounded, in-scope).**

4. **Fix the Distribution Fitting crash (§Test 7).** Debugger-confirmed unhandled
   `std::out_of_range`: add the missing `TimeSeriesSet::append(const TimeSeries<T>&, const std::string&)` overload, guard the name lookups, surface a message
   instead of throwing.
5. **Fix the static-analysis errors (§12a).** The missing return in **both**
   `Conductor::CheckNegativeElements` overloads (live input guard — highest of
   these), the null deref in `CBinary::operator[]`, the closed-`FILE*` writes in
   `GA.cpp`, and the `log()` domain check in `ApplyBoxCoxTransform`. Wire the new
   `lint`/`cppcheck`/`check` tasks into CI.
6. **Fix the inverted "Include all elements" checkbox (G1).** One-line argument
   swap in `formelementinformation.cpp`; the control currently excludes all
   tracers when checked.

**Should-address (quality / release readiness).**

7. **Fix the QXlsx submodule URL** (`.gitmodules` → QtExcel). Blocks every fresh
   clone; the highest-value *quick* fix.
8. **Add a minimal automated test / verified example** exercising core
   apportionment against a known result, and run the static-analysis tasks in CI.
9. **Reconcile release metadata (§14)** — `code.json`/README/`createdeb.sh` point
   at personal GitHub rather than code.usgs.gov.
10. **Address MCMC reproducibility (§10) / RNG thread-safety (§11)** — add a
    seedable, thread-safe PRNG and record the seed.
11. **Reconcile the advertised-but-unbuilt GA (§Test 6 / §12d)** — wire it up or
    drop the claim; note the live MCMC's compile-time dependency on `GA.h`.

**Polish (low severity).**

12. **Repository hygiene** — run/adapt `apply-hygiene-cleanup.sh`; remove the
    `.aip`, the duplicate `gitignore`, and the hardcoded path in the legacy `.pro`.
13. **Documentation** — pin dependency versions, clarify which build system is
    authoritative, and (optionally) fold in the Windows/pixi instructions.
14. **Spelling / per-file license headers / dependency-license inventory**
    (§16, §13).
15. **(Optional) Windows support** — if Windows is to be a supported platform,
    upstream the `Utilities` SuperLU guard (see §3b and
    `BUILDING_ON_WINDOWS_WITH_PIXI.md`, Pitfall 4).

> *On the `Utilities` dependency's fate:* an earlier version of this list
> suggested "vendor or mirror Utilities" to protect release longevity. That
> advice is now **superseded by §13a** — vendoring the AGPL/commercial
> `Utilities` sources into a would-be CC0 release is exactly what §13a flags as
> not viable. The longevity concern (a single-point-of-failure dependency in a
> personal namespace) is real, but it must be resolved *together with* the §13a
> license question, not by vendoring alone.

---

## 9. Repository organization / architecture

The repository is caught mid-transition between two layout conventions, and the
intended GUI-vs-core separation does not hold cleanly at the directory level.

### The intended split (mostly true)

- **`src/` + `include/`** — the modern scientific core, and the only part that
  follows a separated implementation (`src/`) / public-header (`include/`)
  convention. Contains the apportionment engine: `sourcesinkdata`, `conductor`
  (command dispatcher), `GA/`, `MCMC/` (header-only `.hpp`),
  `elemental_profile(_set)`, `concentrationset`, `parameter`, `observation`,
  `interface`, `cmbdistribution`. This part is coherent and well-organized.
- **Repository root** — the older, flat Qt/qmake-style layout (each `.cpp`
  beside its `.h`). Most root files are genuinely GUI: `mainwindow`, the
  `*dialog`/`*window`/`*form` classes, Qt table models/delegates
  (`elementstablemodel`, `selectsampledelegate`), custom widgets, and the
  plotting layer.

### Where the split leaks (review-relevant)

1. **Non-GUI core also lives at root.** The `cmb*` family (`cmbvector`,
   `cmbvectorset`, `cmbvectorsetset`, `cmbmatrix`, `cmbtimeseries(set)`) are
   data-structure/math types, not GUI — as are `multiplelinearregression(set)`,
   `range`/`rangeset`, `contribution`, and `results`/`resultitem`. Root is
   therefore "GUI **plus** everything predating the `src/` reorganization,"
   not "GUI only."
2. **Two header conventions coexist:** `src/` uses a parallel `include/` tree;
   root keeps headers beside sources.
3. **Inconsistent file naming** (suggests history / multiple contributors):

   - Mixed case: `FilePushButton.*`, `GeneralChartPlotter.*` (PascalCase) vs
     `filebrowserpushbuttom.*` (lowercase, and a misspelling — "pushbuttom").
   - Apparent duplication: two file-button classes (`FilePushButton` and
     `filebrowserpushbuttom`) and overlapping chart layers (`generalchart` +
     `generalplotter` at root alongside `GeneralChartPlotter`).
4. **Orphaned / stray files (verified):**

   - `drand.cpp` sits at root with no adjacent header (its header is
     `include/drand.h`) and is **not referenced in `CMakeLists.txt`** — likely
     dead code.
   - `src/GA/GA_impl.cpp` is a 2-byte stub, and `src/GA/GA_impl` is a 0-byte
     extensionless file.
   - `testmcmc.*` (a non-functional test stub) sits at root among GUI files.
5. **"CMB" lineage.** The `cmb*` prefix and artifacts like `CMBSource.pro`,
   `CMB_Source.aip`, and `CMBSource_en_US.ts` show the project's earlier name
   (CMB = Chemical Mass Balance). SedSAT3 is a rename; the flat root layout is
   inherited from that predecessor, which is why the newer `src/include` core
   reads as a later addition bolted onto the original tree.

### Assessment

This is a maintainability/reviewability observation, not a functional defect —
`CMakeLists.txt` enumerates every file explicitly regardless of location, so
the flat layout carries no build cost. However, a reviewer or newcomer cannot
infer the architecture from the directory structure, because the GUI/core
boundary does not map to the folder boundary.

**Optional recommendation (not required for correctness):** finish the
migration — move the non-GUI `cmb*` / regression / range / results core into
the scientific core, and move the Qt GUI sources into their own target
subdirectory as well, so the repository root is left holding essentially just
the top-level `CMakeLists.txt`. Also normalize file-name casing and remove the
orphans (`drand.cpp`, `GA_impl`, the test stub). This would make the GUI/core
separation hold at the directory level and materially improve navigability. The
specific target layout to aim for is Qt's documented one, described next.

**What Qt itself recommends (authoritative — the layout to aim for).** Qt's
official CMake getting-started documentation, in its *"Structuring projects"*
section, recommends that **both application/GUI sources and core sources live
under `src/<target>/`** (its example moves `main.cpp`, `mainwindow.{cpp,h}`, and
`mainwindow.ui` into `src/app/`), leaving the repository **root holding only the
top-level `CMakeLists.txt`** — whose job is the overall project setup plus
`add_subdirectory()` calls — with each target getting its own `CMakeLists.txt`
in its subdirectory. The documented rationale is that *"projects that contain
more than just one target will benefit from a clear project file structure,"*
and that the subdirectory layout lets a shared core library be built once and
linked by both it and the GUI — which is exactly SedSAT3's core-plus-GUI shape.
So the end state to aim for is GUI *and* core each under `src/<target>/` (e.g.
`src/app/`, `src/core/`) with a thin root `CMakeLists.txt` — **not** a flat root
holding the GUI.
Source: Qt 6 documentation, *Getting started with CMake → Structuring projects*
(`https://doc.qt.io/qt-6/cmake-get-started.html`).

> *Scope note (unchanged):* this remains a **maintainability** recommendation,
> not a defect — the current flat layout builds correctly. The reason to treat
> it as worthwhile anyway: if SedSAT3 is to be a USGS-maintained product going
> forward, adhering to the Qt maintainers' own documented project conventions is
> a low-cost investment in long-term reviewability and contributor onboarding.
> *(Note on scope limits: Qt's example clearly establishes the principle —
> sources under `src/`, one subdirectory per target, a thin root
> `CMakeLists.txt`. It names `src/app/` for the application; the exact
> subdirectory name for a core library (`src/core/`, `src/lib/`, etc.) is the
> project's choice and is not prescribed by that page.)*

---

## 10. Reproducibility and random-number handling (dedicated finding)

**Standard being applied:** USGS SM 502.11 requires documentation/software that
lets a knowledgeable third party "evaluate, use, and **reproduce** the results."
For software with stochastic methods, reproducibility requires that randomness
be controllable — typically via an explicit, user-settable PRNG seed, with the
seed recorded in the output so any run can be replayed.

**Current behavior (verified in source):**

- The Bayesian/MCMC sampler seeds C's global RNG from the wall clock:
  `srand(int(time(NULL)) ^ omp_get_thread_num() + kk);`
  (`include/MCMC/MCMC.hpp`, ~line 433). A second clock-based seed appears in
  `Utilities/TimeSeries.hpp` (~line 443: `seed = std::time(nullptr)`), and a
  legacy `drand.cpp`/`include/drand.h` defines its own fixed seed constants.
- There is **no user-facing seed control** and the seed used is **not recorded**
  in any output. Because it is derived from `time(NULL)` and discarded, a given
  result cannot be replayed even in principle.
- Randomness is **not centralized** — multiple independent seeding sites exist
  across MCMC, TimeSeries, GA, and the legacy drand.

**Consequence:** repeated Bayesian runs on identical input are not reproducible,
and neither the author nor a reviewer can reconstruct the exact run behind any
reported figure.

**Measured framing (important):** For a converged MCMC sampler, the *scientific
summaries* (posterior means, credible intervals) may be stable across seeds to
within sampling noise, so run-to-run differences are not automatically a bug.
The defensible, seed-independent finding is narrower and airtight: **the
software provides no mechanism to reproduce a specific run and does not record
the seed used** — a reproducibility-tooling gap against the SM 502.11
"reproduce the results" standard.

**Recommendation (three parts):**

1. Route all stochastic methods through a **single, seedable, thread-safe PRNG**
   (e.g., per-thread `std::mt19937` derived deterministically from one master
   seed, or GSL's `gsl_rng` per thread — GSL is already a dependency). Retire
   the scattered `srand`/`rand` and legacy `drand`.
2. **Expose the seed** as a user option (GUI/config).
3. **Record the seed** used into the results/output, so even an unseeded default
   run is reproducible after the fact.

**Cross-reference:** see Test 1 in `REVIEWER_TEST_PLAN.md` for the GUI procedure
that demonstrates this behavior.

---

## 11. Parallelism (OpenMP) — is it warranted, and is it done well?

**What is parallelized (verified):** OpenMP targets the *algorithm* dimension,
not the data dimension:

- **MCMC:** `#pragma omp parallel for` over the Markov **chains**
  (`include/MCMC/MCMC.hpp` ~line 437). Running independent chains in parallel is
  standard, legitimate MCMC practice.
- **GA:** `#pragma omp parallel for` over the **population**
  (`include/GA/GA.hpp` ~line 376; also `src/GA/GA.cpp` ~line 284). Evaluating
  population fitness in parallel is textbook.

So the parallelism is not about large datasets (sediment-fingerprinting inputs
are small). It addresses the compute cost of the *methods* — many MCMC samples
or GA generations, each requiring a full forward-model evaluation. On that
basis the choice to parallelize is defensible and conventional.

**Is it *needed*?** A fair cost/benefit question for the review. Parallelism
adds a build dependency (OpenMP), threading complexity, and — as implemented —
the reproducibility and portability problems below, in exchange for a speedup
whose user-perceptible benefit on small datasets is undocumented. Not "wrong,"
but the added complexity is unexplained and unmeasured. Recommend the developers
document the rationale and expected benefit, or make it optional.

**Is it *done well*? Implementation issues (verified):**

1. **Platform-divergent seeding.** The per-thread `srand` seeding sits inside a
   `#ifdef WIN64` block (`MCMC.hpp` ~line 429), so RNG initialization differs on
   Windows vs. other platforms — a portability red flag layered on top of the
   reproducibility issue in Section 10.
2. **Non-thread-safe RNG (data race — code-inspection finding).** C's global
   `rand()`/`srand()` are called from within an OpenMP parallel region
   (`MCMC.hpp` ~lines 429–440): threads race on the generator's shared internal
   state. The likely effect is **silent statistical degradation** of the random
   stream (correlations, repeated draws), not a crash — glibc/MSVC `rand()` will
   not segfault on this race. The per-thread `srand(... ^ omp_get_thread_num())`
   also fails its own intent: all threads write one global state, so per-thread
   seeding accomplishes nothing.
   This is reported as a **static/code-inspection finding, not a runtime test
   result.** It is timing-dependent and cannot be reliably reproduced by
   black-box testing, and it is *masked* by the non-reproducibility issue in
   Section 10 (degraded randomness is indistinguishable from normal run-to-run
   variation when the seed is already random). A per-thread PRNG (the Section 10
   recommendation) fixes the race and the reproducibility gap together.
3. **File I/O inside the parallel region.** The GA opens/writes
   `detail_GA.txt` from within the parallel loop, serialized by
   `#pragma omp critical` (`GA.hpp` ~line 408). This partially defeats the
   parallelism and is a performance smell.
4. **Debug logging in the hot loop.** `qDebug()` per step/chain inside the
   parallel region (`MCMC.hpp`) degrades performance and produces interleaved
   output.

**Summary:** the decision to parallelize is reasonable; the implementation has
correctness (thread-safety), portability (`#ifdef WIN64` seeding), and quality
(in-loop I/O and logging) issues, and the justification for the added
complexity is undocumented.

---

## 12. Static analysis (tooling now wired up; concrete findings)

**Context.** The project already declared `clang-tools` (clang-tidy) and
`cppcheck` under `[dependencies]` in `pixi.toml`, but no task invoked them, so
the analyzers were installed-but-unreachable — an "advertised vs. actual"
mismatch of the same flavor as the GA-methods gap (Test 6). During this review
the tooling was wired up so the declared analyzers can actually be run.

**Tooling added (`pixi.toml`).**

- `configure` now emits a compilation database
  (`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) so clang-tidy has accurate include
  paths and flags.
- `lint` — runs clang-tidy over the scientific core (`src/` + `src/GA/`),
  depends on `configure`.
- `cppcheck` — runs cppcheck over `src/` + `include/` (standalone; no
  compilation database needed).
- `check` — convenience task running both analyzers.

**Packaging workaround worth recording.** The conda-forge `win-64` cppcheck
build (2.21.0) bakes its `FILESDIR` to the build server's path
(`D:/bld/...`), so on Windows it aborts at startup unable to find its bundled
`std.cfg`. Neither a runtime `FILESDIR` environment variable nor CLI flags
(`--data-dir`, `--library`) redirect that startup lookup in this version. The
reliable fix is cppcheck's executable-relative fallback (`<exedir>/cfg`): a
hidden helper task `_cppcheck-stage-cfg` stages the env's cfg files there. It
is idempotent and self-heals after `pixi clean` / a fresh env install, and the
staged copy lives under `.pixi/` (gitignored). This is a Windows-only
packaging quirk; Linux/macOS cppcheck packages are unaffected.

**Scope of the run.** cppcheck was executed against the scientific core (14
translation units in `src/` + `include/`), checked to 100%. clang-tidy is
wired up but a full clang-tidy pass requires the MSVC developer shell (to
generate the compilation database) and was not exhaustively run here; the
cppcheck results below are the concrete, reproduced output. These are
**tool-generated static-analysis findings**, verified against the cited source
lines.

### 12a. Errors (verified against source)

1. **Null-pointer dereference — `src/GA/Binary.cpp:139`
   (`CBinary::operator[]`).** The out-of-range branch returns `*p` where
   `int *p = 0;`:

   ```cpp
   int& CBinary::operator[](unsigned int i) {
       int *p = 0;
       if (i < Digit.size()) return this->Digit[i];
       else                  return *p;      // dereferences null on OOB access
   }
   ```

   An out-of-bounds index yields a guaranteed null dereference (crash) rather
   than a checked error.
2. **Use of a closed `FILE*` — `src/GA/GA.cpp:308–309`.** The stream `FileOut`
   is `fclose`d at ~line 301, then written again with `fprintf(FileOut, ...)`
   at line 308 and `fclose`d a second time at line 309 — writing to and closing
   an already-closed handle (undefined behavior). This sits in the same GA
   file-I/O block flagged in Section 11 (item 3) as being inside the parallel
   region.
3. **Missing return on a non-void function — `src/conductor.cpp:213`
   (`Conductor::CheckNegativeElements`, returns `bool`).** When `message == ""`
   the function falls off the end without returning, so the caller receives an
   indeterminate value. This is directly review-relevant: `CheckNegativeElements`
   is the negative-concentration input guard that **Test 4c** in
   `REVIEWER_TEST_PLAN.md` is designed to probe — the guard's "input is clean"
   path returns nothing.

   **Update (second overload, found during Test 7).** There is a *second*
   `Conductor::CheckNegativeElements` overload — the `map<string,vector<string>>`
   form (`src/conductor.cpp:~193`) used on the **Distribution Fitting** path —
   with the **same** missing-return defect: when `message == ""` it also falls
   off the end without returning. So the defect is not a single site but a
   **pattern across both overloads** of this guard. Empirically (Test 7,
   `TEST_RESULTS.md`) the DF path proceeds past this guard into the crash, i.e.
   the indeterminate return happened to be truthy on this MSVC build — again
   *masked, not benign*. Fix both overloads with an explicit `return true;` on
   the clean path.
4. **Possible `log()` of a non-positive value — `src/concentrationset.cpp:386`
   (`ApplyBoxCoxTransform`).** `log(variance_term)` where `variance_term` is
   accumulated from negative contributions; cppcheck flags argument 1 as
   potentially `0` (domain error → `-inf`/`NaN`). Whether it can actually reach
   ≤ 0 in practice is worth the developers' confirmation.

### 12b. Warnings (selected, verified categories)

- **Uninitialized members / incomplete special members.** `class Interface`
  (`src/interface.cpp`) leaves `lowlimit`/`highlimit` uninitialized in the
  constructor, copy constructor, and `operator=`. Similar
  `operatorEqVarError`/`uninitMemberVar` findings in `Observation`
  (`predicted_value`), `Parameter` (`value`), and `element_information`
  (`standard_ratio`, `include/elemental_profile.h`). Uninitialized numeric
  members are a latent source of nondeterministic behavior.
- **`nullPointerOutOfResources`** — many sites in `src/GA/GA.cpp` write to
  `FileOut`/`FileOut1` without checking the `fopen` result, so a failed open
  dereferences null.
- **Performance (informational).** Numerous `returnByReference`,
  `passedByValue`, `useInitializationList`, and `postfixOperator` notes across
  `parameter`, `observation`, `interface`, and `conductor` — low-priority
  cleanup, not correctness issues.

### 12c. Assessment

The static-analysis pass is a net addition to the review: it confirms the
apportionment core compiles cleanly (Section 6) but contains concrete
correctness defects that black-box GUI testing would not reliably surface,
concentrated in the **GA path** (`Binary.cpp`, `GA.cpp`) — the same code that
is not exposed in the menus (Test 6) and whose `GA_impl` stubs were flagged as
orphaned in Section 9. The `conductor.cpp:213` missing-return in the
negative-element guard is the most broadly reachable, since that guard is on
the live input-validation path.

**Recommendations.**

1. Fix the four errors in 12a (the null deref, the closed-file writes, the
   missing return, and the `log` domain check). The missing return is the
   highest priority as it affects the live input guard.
2. Adopt the wired-up `lint`/`cppcheck`/`check` tasks as part of the
   development workflow, and consider running them in CI (the current sole
   GitHub Action only builds documentation — Section 5).
3. Initialize all class members (in initialization lists), which removes a
   class of nondeterminism that compounds the reproducibility concerns in
   Section 10.

### 12d. clang-tidy pass (full run; evidence in `clang-tidy-report.txt`)

A full `pixi run lint` (clang-tidy, LLVM 23.1.1) was subsequently completed from
a VS Developer shell, driven by the new `.clang-tidy` config and the
`compile_commands.json` emitted by `configure`. The raw output is retained at
`REVIEW_COMMENTS/clang-tidy-report.txt` (1,238 lines). Summary:
`Suppressed 3198 warnings (3159 in non-user code…)` — the `HeaderFilterRegex`
correctly kept Qt/Armadillo/system-header noise out — leaving a focused set of
project-level diagnostics plus **8 compiler errors**.

**The 8 "compiler errors" — diagnosed, and one is a real finding:**

- **Missing header `StringOP.h` (`src/GA/GA.cpp:4`).** `GA.cpp` does
  `#include "StringOP.h"` unconditionally, but **that header exists nowhere in
  the repository** (verified by tree-wide search). This is not a
  clang-vs-MSVC artifact — the file is genuinely absent. It means `GA.cpp`
  cannot compile as-is, which is direct corroboration that the GA translation
  units are **orphaned / not part of the active build** (consistent with the
  101-TU build in Section 6 succeeding, and with the orphaned-GA observations in
  Section 9: `GA_impl` stubs, GA absent from `tools.json`). If the GA is ever
  wired up, this include will break the build.
- **`incomplete type 'ProgressWindow'` / `'QCoreApplication'` (`include/GA/GA.hpp`).**
  The header-only GA code accesses members of forward-declared types without
  their full definitions visible at that point. This is partly a stricter-parse
  effect and partly genuine include-hygiene fragility in the GA headers.

**Consequence for interpretation:** because these errors occur *during parsing*
of the GA files, clang-tidy's analysis of the GA code is incomplete/unreliable —
so the **cppcheck** findings (12a/12b), which parsed those files standalone,
remain the authoritative GA results. The errors did **not** poison the rest of
the run: `sourcesinkdata.cpp` and the other core TUs were fully analyzed.

**Valid clang-tidy warnings (corroborating and additive):**

- **Uninitialized variables (`cppcoreguidelines-init-variables`)** — numerous in
  `sourcesinkdata.cpp`'s statistical/DFA routines: `mean_delta`,
  `mean_base_concentration`, repeated `std1/std2/mean1/mean2`,
  `degrees_of_freedom`, `best_wilks_lambda`, `best_f_test_p_value`. A **second
  tool independently confirming** the uninitialized-value theme from 12b, now in
  the statistics code — reinforces the reproducibility concern (Section 10).
- **Performance (`performance-unnecessary-value-param`, `performance-for-range-copy`)** —
  `string`/`map` parameters and loop variables copied rather than taken by
  const reference (e.g. `target_sample`, `arguments`). Low priority, real,
  trivially fixable.
- **`bugprone-implicit-widening-of-multiplication-result`** — `int` multiplies
  widened to `size_t` in index math; minor, occasionally a real overflow risk.

**Added recommendation:** remove or repair the orphaned GA code — either delete
the dead GA translation units or, if the GA is to be revived (it is advertised
in the README, Test 6), supply the missing `StringOP.h`, fix the GA header
includes, and wire the commands into `tools.json`.

**Caveat — the GA header is not cleanly severable.** `include/MCMC/MCMC.h`
does `#include "GA.h"`, so the **live MCMC sampler depends on the GA header**.
The GA code is therefore not isolated dead weight that can simply be deleted:
removing/repairing the GA must account for the MCMC's compile-time dependency on
`GA.h`. This also explains why the GA's broken includes surfaced during
clang-tidy parsing of MCMC-adjacent code. (Relevant to the §15 disposition of
the GA path as well.)

---

## 13. License conflict with USGS release policy (release blocker)

> **See also §13a** (immediately following this section) for the **most
> consequential** license finding: the first-party `Utilities` core carries
> **AGPL-3.0 + commercial** ("OpenHydroQual") headers asserting private copyright,
> which is sharper than the GPL-library issue described here and ties directly to
> the §15 authorship/authority question.

**Standard being applied.** USGS's *Distribution of USGS Code* guidance
(https://www.usgs.gov/products/software/software-management/distribution-usgs-code)
directs that USGS software should, by default, be released under an
**unencumbered open-source license**, with the acceptable set being:

- **CC0 1.0** (public-domain dedication) — the default, appropriate when the
  software is "either completely original, or using other software also with
  the CC0 license";
- **MIT/X11** — for jurisdictions that do not recognize public-domain
  dedication;
- **The Unlicense** — to opt out of copyright altogether.

> *Provenance / confidence note:* the acceptable-license list above is quoted
> from USGS guidance relayed to this reviewer; the live policy page was not
> directly retrievable during review (HTTP 403) and the exact current wording
> should be confirmed with the USGS software-release coordinator. The
> license-compatibility mechanics below (GPL copyleft propagating through
> linking/embedding) are independent of that wording and are well established.
> SM 1100.6, sometimes cited here, is a *Publishing*-series policy about
> reproducing third-party **copyrighted material** (figures, photos, quoted
> text) in information products; it is at best a loose analogy for linked
> source dependencies, not the governing rule.

> *Open escalation (as of 2026-10-15 — no response received).* The reviewing
> party has raised this exact ambiguity with the policy owners on the internal
> **CDI-Software** Teams channel (General) — a repost of a reply to David Donato,
> addressed to Carl Schroedl (DOI OCIO) and others who may know the answers —
> and, as of this date, **has heard nothing back.** The questions posed there
> are the same ones this review keeps hitting, because real software is rarely
> 100% federally-authored:
>
> 1. **Vendored permissive module (MIT/BSD/Apache).** When USGS code includes a
>    permissively-licensed snapshot whose author still holds copyright, is the
>    intended pattern a top-level CC0/Unlicense/MIT for the federal work **plus a
>    NOTICE / third-party-licenses file** preserving the dependency terms? Is
>    there approved boilerplate, and does it need a Solicitor consult each time or
>    is it a documented, repeatable pattern? *(The questioner points to a working
>    attempt — `swb2/THIRD_PARTY_LICENSES.md` — as a candidate model.)*
> 2. **Released examples.** Are there existing USGS repos that did exactly this
>    (federal code + a permissive dependency) whose structure could be copied?
> 3. **The copyleft case (GPL/LGPL/AGPL).** This feels categorically different
>    from the permissive case because copyleft can attach obligations to the
>    *combined work*. Is there standing guidance or precedent, or is it genuinely
>    case-by-case? And is there a recognized distinction between USGS-generated
>    source (CC0), the GPL dependency source, and the resulting compiled binary?
>
> The underlying ask — *where is the line between "documented pattern I can just
> follow" and "genuinely needs the Solicitor"* — is **unresolved** and is exactly
> the uncertainty that makes §13/§13a hard to close from the code-review side.
>
> *What FAQ #137 actually says (and why it does not resolve this case).* The
> David-Donato reference in that thread is USGS FAQ #137, *"How must USGS
> scientific software information products be licensed?"* Its full text was
> supplied by the questioner. The operative points:
>
> - It defines an **"original work of the U.S. Government"** narrowly: *"authored
>   solely by Federal employees and does not include any pre-existing third-party
>   works,"* which is in the public domain (17 USC 105a) and should carry a
>   public-domain dedication (CC0).
> - For **everything else** it gives no pattern and no acceptable-license list —
>   it routes the matter out: *"For other situations (such as pre-existing work or
>   an original work of the U.S. Government incorporating pre-existing third-party
>   works) or to apply different licenses, please consult with the Department of
>   the Interior Office of the Solicitor."*
>
> **This is the key takeaway for SedSAT3.** The FAQ addresses only the clean
> 100%-federal case; it says nothing usable about MIT/Unlicense, nothing about
> copyleft (GPL/LGPL/AGPL) dependencies, and offers no repeatable NOTICE/third-
> party pattern. SedSAT3 is squarely in the FAQ's **"other situations"** bucket —
> it incorporates pre-existing third-party works (the GPL libraries of §13) and
> pre-existing non-federal author code (the AGPL `Utilities` core of §13a) — so by
> the FAQ's own terms it is a **Solicitor consult**, not a self-service pattern.
> That answers the questioner's "where is the line" question for *this* codebase
> (it falls on the Solicitor side) while leaving the broader
> documented-pattern-vs-Solicitor question — the point of the Teams escalation —
> still unanswered for the common cases.
>
> **Teams thread (internal, CDI-Software → General):**
> https://teams.microsoft.com/l/message/19:b34ccbb114b545faafb03ef7af656fff@thread.skype/1789583918259?tenantId=0693b5ba-4b18-4d7b-9341-f32f400a5494&groupId=01927172-3614-4d93-9cbe-6e7c31ad371e&parentMessageId=1785285237367&teamName=CDI-Software&channelName=General&createdTime=1789583918259
>
> *Solicitor routing (update, 2026-10-15).* Consistent with the FAQ #137
> "other situations → consult the Solicitor" routing above, the reviewing party
> reached out to the DOI Office of the Solicitor (Shannon Bremer), who
> **suggested** — helpfully, but as a pointer rather than a definitive routing —
> that the question would likely belong with the Division of General Law's
> Acquisitions and Intellectual Property Branch, and that (following Nancy Lewis'
> June departure) a reasonable person to try might be the Acting Assistant
> Solicitor for that branch, Bill Blake (william.blake@sol.doi.gov). This is a
> *lead to follow*, not a confirmed owner of the question, and it has not yet been
> pursued.
>
> *The actual difficulty, stated plainly.* This exchange illustrates the core
> frustration behind the whole §13/§13a/§15 thread: the standing guidance and
> every referral so far converge on **"contact the Solicitor,"** but the next
> steps past that — *which* office or branch actually owns software-license
> questions of this kind, through *what* venue or process, and who makes the
> call — are **not clearly defined.** Each hop (FAQ → Solicitor → "probably this
> branch, maybe this person") is a suggestion to the next hop rather than an
> authoritative hand-off. So the open item is not merely "ask the Solicitor"; it
> is that **the path and the responsible party remain unclear even after asking.**
> That ambiguity is itself a finding worth surfacing to the release coordinator,
> because it is what makes §13/§13a/§15 hard to actually close.

**The conflict.** SedSAT3 is released under **GPL-3.0** (`LICENSE.md`;
`code.json` status "Development"). GPL-3.0 is **not** in the USGS-acceptable
set, and cannot be — GPL is a *copyleft* license that uses copyright to impose
share-alike/source-disclosure conditions, which is the opposite of the
"maximally unencumbered / public domain" intent of the CC0 default for
U.S. Government work.

**Why this is not a one-line fix (verified dependency licenses).** SedSAT3
cannot simply swap `LICENSE.md` to CC0/MIT/Unlicense, because it links and
embeds **GPL** dependencies whose copyleft propagates to the distributed whole.
Licenses were read directly from the tree:

| Component                                         | Role                                                               | License                                               | Evidence                                                                                                                                                                                                         |
| ------------------------------------------------- | ------------------------------------------------------------------ | ----------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SedSAT3 own code (GA, Levenberg–Marquardt, etc.) | first-party                                                        | *no per-file notice*; repo `LICENSE.md` = GPL-3.0 | source inspection — the GA (`CBinary`/`CGA`, MFC-era origin) and LM (`OneStepLevenberg_Marquardt`) are **original author code**, not borrowed                                                       |
| **GSL**                                     | linked                                                             | **GPL-3.0**                                     | `#include <gsl/gsl_multifit.h>`, `gsl_multifit_wlinear` in `multiplelinearregression.cpp`                                                                                                                  |
| **qcustomplot6**                            | vendored (`thirdparty/`)                                         | **GPL**                                         | header: "Copyright (C) 2011-2022 Emanuel Eichhammer … GNU General Public License"                                                                                                                               |
| **Qt Charts**                               | linked add-on (`QT_CHARTS_LIB`, `USE_QCHARTS`, `qt6-charts`) | **GPLv3** (or commercial)                       | Qt docs, `qtcharts-index.html`: header "Qt Charts \| Commercial or GPLv3"; *"available under the GNU General Public License, version 3."* Also **deprecated since Qt 6.10** (superseded by Qt Graphs). |
| Qt core (Widgets/Gui/Core/PrintSupport)           | linked                                                             | **LGPLv3** — *not a blocker*                 | Qt: LGPLv3 is Qt's "primary open-source license"; dynamic linking keeps your code unaffected                                                                                                                     |
| QXlsx                                             | submodule                                                          | MIT                                                   | `thirdparty/QXlsx/LICENSE`                                                                                                                                                                                     |
| Utilities                                         | submodule                                                          | Apache-2.0                                            | `Utilities/LICENSE.md`                                                                                                                                                                                         |

The permissive dependencies (QXlsx = MIT, Utilities = Apache-2.0) are
compatible with any target license and are **not** the obstacle. Neither is
**Qt core** (Widgets/Gui/Core/PrintSupport): Qt's *primary* open-source license
is **LGPLv3**, and — as the FSF FAQ confirms below — LGPL is precisely the
license that permits (dynamic) linking **without** propagating copyleft to the
application's own code. **The GUI does not have to be removed.** SedSAT3 already
dynamically links Qt (its pixi/DLL deployment does exactly this), which is the
condition Qt requires for the LGPL shield (static linking would forfeit it).

The obstacle is the **three GPL components**:

1. **GSL** — GPL-3.0; used for a single call (`gsl_multifit_wlinear`).
2. **qcustomplot6** — GPL; vendored plotting library.
3. **Qt Charts** — GPLv3-or-commercial for open-source use (verified from Qt's
   own docs), and additionally **deprecated since Qt 6.10**.

Any one of these, being linked/embedded and redistributed, forces the combined
SedSAT3 distribution to be GPL. Note the entanglement is concentrated in the
**plotting layer**: SedSAT3 carries *two overlapping GPL charting libraries*
(qcustomplot6 **and** Qt Charts). The barrier is not SedSAT3's own code (which
is original and could be CC0-dedicated freely), nor the Qt GUI framework
(LGPLv3) — it is these three add-on/numerical libraries.

**Authoritative basis for the linking analysis (FSF GPL FAQ).** Because the
"we only *use* GSL, we don't *modify* it" and "dynamic linking avoids the GPL"
arguments come up frequently, the relevant mechanics are grounded here directly
in the license author's own FAQ (https://www.gnu.org/licenses/gpl-faq.html;
the FSF notes these are explanatory, not legal advice):

- **Linking — static or dynamic — creates a covered combined work.** *"Does the
  GPL have different requirements for statically vs dynamically linked modules
  with a covered work? **No.** Linking a GPL covered work statically or
  dynamically with other modules is making a combined work based on the GPL
  covered work. Thus, the terms and conditions of the GNU General Public
  License cover the whole combination."*
- **Using (not just modifying) a GPL library triggers copyleft.** *"If a library
  is released under the GPL (not the LGPL), does that mean that any software
  which uses it has to be under the GPL or a GPL-compatible license? **Yes,
  because the program actually links to the library.** As such, the terms of the
  GPL apply to the entire combination."*
- **LGPL is the license specifically designed to permit linking without
  propagating.** This is why **Qt core (LGPLv3) is fine** when dynamically
  linked. But **GSL, qcustomplot6, and Qt Charts are full GPL, not LGPL**, so
  this escape does **not** apply to them — dynamic linking does not help for
  those three.

**Common misconception to preempt (documented for the developers):** it is
*not* the case that "importing/using a GPL library without editing its source
leaves your own code unaffected." Per the FAQ quotes above, mere linking forms
the combined work; modification is not the trigger. This is the single point
most often gotten wrong in discussions of this issue, so it is called out
explicitly here.

**Resolution paths (a decision, not an edit — requires the release authority).**

1. **Replace the three GPL dependencies with permissively-licensed
   equivalents:**

   - **The plotting layer (both `qcustomplot6` and Qt Charts).** These are
     *both* GPL, so one cannot substitute for the other — the fix is to move
     charting onto a permissively-licensed library (there are MIT/BSD Qt
     charting options) or a self-drawn `QPainter`/`QGraphicsView` approach on
     LGPL Qt core. Qt Charts is **deprecated since Qt 6.10** regardless, so
     migrating off it is warranted on maintenance grounds alone. Consolidating
     to a single, permissive plotting path removes two GPL dependencies at once.
   - **GSL** → replace the single `gsl_multifit_wlinear` call (the only GSL use
     found) with a permissive equivalent — Armadillo (already a dependency) or
     Eigen. GSL is GPL-only, so alternate licensing of GSL itself is not
     available.
   - *Retain* Qt core under **LGPLv3** with dynamic linking (no change needed).

   **Plotting-library options evaluated (licenses verified this review):**

   | Option                                                | License                                                                                                     | Verdict                                                                                                         |
   | ----------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
   | Qt Charts (current)                                   | **GPLv3** or commercial                                                                               | ✗ blocker; also deprecated since 6.10                                                                          |
   | qcustomplot6 (current)                                | **GPL** or paid commercial                                                                            | ✗ blocker                                                                                                      |
   | **Qt Graphs** (Qt's successor to Charts)        | **GPLv3** or commercial (verified: `qtgraphs-index.html` header "Qt Graphs \| Commercial or GPLv3") | ✗ *not* a fix — same GPL trap, so migrating Charts→Graphs solves deprecation but **not** the license |
   | **QWT** (Qt Widgets for Technical Applications) | **LGPL 2.1 + added permissions** (verified: qwtlicense.html)                                          | ✓ **strong candidate** — see below                                                                      |
   | Self-drawn on `QPainter`/`QGraphicsView`          | **LGPLv3** (Qt core only)                                                                             | ✓ zero new dependency; most implementation effort                                                              |

   **On QWT:** its license is LGPL 2.1 with three *additional permissions*
   (relaxations, not restrictions): subclassing Qwt widgets is not a derivative
   work; **static linking does not require providing your application's
   source**; and you need only an attribution line, not the full license text.
   These carve-outs mean QWT imposes **no meaningful copyleft on SedSAT3's own
   code** — SedSAT3 could be CC0 while linking QWT, with a one-line credit
   ("… based in part on the work of the Qwt project"). QWT is also purpose-built
   for scientific/technical plotting, a good functional fit. Caveat: QWT itself
   remains under its LGPL-based terms and belongs in the dependency inventory;
   whether "LGPL-2.1-with-added-permissions" fully meets the letter of the USGS
   CC0/MIT/Unlicense preference is a call for the release coordinator — but it
   is categorically **not** a full-GPL blocker.

   **Recommendation:** consolidate the two GPL charting libraries onto a single
   permissive path — either QWT (least code, verify license acceptance) or a
   self-drawn `QPainter`/`QGraphicsView` layer on LGPL Qt core (most code, zero
   new dependency and zero residual license question). Both remove qcustomplot6
   **and** Qt Charts at once.
2. **Seek a documented policy exception/waiver** to release under GPL where a
   GPL dependency is genuinely unavoidable. This runs through the **internal USGS
   software-release process** (release coordinator / Science Publishing Network).
   It is a *different flavor* of question from classic copyright-permission work
   (where the Solicitor obtains rights to others' copyrighted material): here the
   GPL libraries are already freely usable, and the live question is which
   outbound license SedSAT3 adopts given those dependencies. **However** — see the
   "Open escalation / FAQ #137" note above — FAQ #137 routes *any* work that
   "incorporates pre-existing third-party works," or that seeks "to apply
   different licenses," to the **DOI Office of the Solicitor**. By that plain
   language this case likely does reach the Solicitor after all. (An earlier draft
   of this note asserted flatly that it was "not a Solicitor matter"; the FAQ text
   the questioner later supplied does not support that absolute claim, so it is
   softened here.) Treat the Solicitor involvement as *likely-required*, confirmed
   via the release coordinator, rather than excluded.
3. **Accept that it cannot be CC0/MIT/Unlicense as currently constituted** and
   record that constraint explicitly.

**Severity and realistic effort (so this is not read as worse than it is).**
This is a genuine release blocker, but a *bounded* one. It is **not** an
Office-of-the-Solicitor escalation, and it does **not** require removing the
GUI. SedSAT3's own algorithms are original and CC0-ready today; the work is a
contained refactor of three add-on/numerical dependencies (one plotting layer
covering two libraries, plus one GSL call), after which the USGS-preferred
**CC0** becomes available. The reason to raise it early is that it is far
cheaper to resolve during review than after a release attempt stalls.

**Related attribution gaps (lower severity, same domain).**

- **No per-file copyright/license notices** in the first-party `src/` and
  `include/` sources. For any final license (GPL today, CC0/MIT later), USGS
  practice and general open-source convention call for a short header notice
  per source file.
- **No dependency-license inventory** shipped with the release. The table above
  is the kind of NOTICE/third-party-licenses manifest the release should carry,
  recording each bundled/linked component and its license (this also satisfies
  the "mark copyrighted third-party material" spirit that the Teams guidance
  invoked SM 1100.6 for).

**Priority.** This is a **release blocker**, not cleanup: a USGS release cannot
proceed under a non-conforming license without either resolving the GPL
dependency chain or obtaining an explicit exception. It should be raised with
the software-release coordinator early, because path (1) is a non-trivial
engineering change and path (2) is a process with lead time.

---

## 13a. First-party AGPL dependency: the `Utilities` core is author-copyrighted, commercially-licensed OpenHydroQual code (elevated release blocker; ties §13 ↔ §15)

**This is the most consequential license finding, and it is sharper than the
§13 GPL-library issue.** It was surfaced incidentally while reading `Utilities`
source during the Distribution Fitting crash investigation
(`TEST_RESULTS.md` Test 7).

**Verified facts (from the source tree and the upstream repository):**

- **Per-file AGPL headers in the `Utilities` core.** At least the
  `TimeSeries` / `TimeSeriesSet` family carry a banner reading:

  > *"OpenHydroQual - Environmental Modeling Platform / Copyright (C) 2025 Arash
  > Massoudieh … OpenHydroQual is free software: you can redistribute it and/or
  > modify it under the terms of the **GNU Affero General Public License** …
  > If you use this file in a commercial product, you must purchase a commercial
  > license. Contact arash.massoudieh@enviroinformatics.co …"*
  >

  Confirmed by text scan in: `Utilities/TimeSeries.h`, `Utilities/TimeSeries.hpp`,
  `Utilities/TimeSeriesSet.h`, `Utilities/TimeSeriesSet.hpp` (4 files). The scan
  was keyword-based ("Affero"/"OpenHydroQual"/"commercial license") and is a
  **lower bound** — other `Utilities` files may carry the same header, a
  different one, or none; a full per-file audit is warranted.
- **Upstream confirms dual licensing + private copyright.** The OpenHydroQual
  repository (https://github.com/ArashMassoudieh/OpenHydroQual) states verbatim:
  *"OpenHydroQual is released under the GNU AGPLv3 license **for academic and
  research use**,"* and, under **Commercial Licensing**, *"If you intend to use
  OpenHydroQual in a proprietary software or commercial product, you must acquire
  a separate commercial license. … Dr. Arash Massoudieh, EnviroInformatics,
  LLC."* So this is **dual-licensed, author/EnviroInformatics-owned commercial
  software** (AGPL-3.0 OR paid commercial), not an unencumbered library.
- **Direct contradiction with the submodule's stated license.** The `Utilities`
  submodule ships `Utilities/LICENSE.md` = **Apache-2.0** (as recorded in the
  §13 dependency table). But the per-file headers in that same submodule assert
  **AGPL-3.0 + commercial**. These cannot both govern: the top-level Apache-2.0
  license file and the per-file AGPL/commercial notices are **inconsistent**, and
  the §13 table's "Utilities = Apache-2.0" row is therefore not reliable.

**Why this is worse than the §13 GPL-library finding:**

1. **AGPL-3.0 is the strongest mainstream copyleft** — beyond GPL, it extends
   copyleft to *network interaction* (its §13 network clause), and is the
   furthest of all from the USGS CC0/public-domain default.
2. **It is first-party core code, not a swappable third-party library.** Unlike
   GSL / qcustomplot6 / Qt Charts (§13, replaceable), `Utilities` types
   (`CMBTimeSeries`/`TimeSeriesSet`, etc.) are woven through SedSAT3's numerical
   core. It cannot be substituted without a substantial re-implementation.
3. **It collides with the CC0 premise itself (this is the §15 link).** The USGS
   CC0 default rests on the software being a public-domain **U.S. Government
   work** (federal-employee authorship). Here the author asserts **private
   copyright** and **sells commercial licenses** to the very code underlying
   SedSAT3. The same code cannot simultaneously be (a) EnviroInformatics's
   copyrighted, commercially-licensed IP and (b) a public-domain USGS work under
   CC0. This converts the §15 "authorship/authority unclear" question into a
   concrete "**core dependency is asserted private IP**" finding.

**How the needle can be threaded (a release-authority decision, NOT a
code-review determination — routes to the software-release coordinator / Science
Publishing Network and, given asserted private copyright, likely the Office of
the Solicitor):**

1. **Author grants USGS a suitable license / CC0 dedication for the
   SedSAT3-used `Utilities` sources.** As the copyright holder, Massoudieh/
   EnviroInformatics *may* dual-license their own code again — issuing USGS a
   permissive/CC0 grant for the specific files SedSAT3 needs, independent of
   OpenHydroQual's AGPL/commercial terms. Cleanest path **if he agrees** and it
   is captured **in writing on file** with the release. Tension: it may compete
   with his own commercial product.
2. **Establish USGS's right to release via a contribution / cooperative-agreement
   / CRADA / contract instrument** covering the non-federal author's code (the
   §15 "under what authority" question). Such an instrument can define the
   outbound license USGS may use.
3. **Re-implement the AGPL portions as clean permissively-licensed or federal
   work** so the released product contains none of the author-copyrighted AGPL
   code — the path that genuinely restores CC0 eligibility, at significant
   engineering cost.
4. **Documented USGS exception to release under the inherited copyleft.** An
   *AGPL* USGS release would be highly unusual and must also be reconciled with
   the other GPL dependencies (§13); a coordinator/Solicitor call.

**What is NOT viable:** shipping AGPL-headered, commercially-licensed code inside
a "USGS software / public domain / CC0" release. The per-file AGPL notices and
the "purchase a commercial license" line are explicit assertions of private
copyright that a CC0/MIT release would directly contradict.

**An honesty note on AGPL and USGS policy (what we could and could not
establish).** We want to be candid about the limits of what this review could
confirm. (This is a *different* question from the "NOT viable" line just above:
that line is about an internal contradiction — AGPL-headered code cannot
simultaneously be labeled public-domain CC0; the question here is the separate
one of whether *AGPL itself* is an acceptable outbound USGS license.) The USGS
guidance we were able to locate expresses a *preference* for an unencumbered set
(CC0 / MIT / Unlicense; see §13) — it does **not** contain any statement that
affirms AGPL as acceptable, nor one that explicitly forbids it. We searched for a
definitive ruling and did not find one. So the defensible finding is **not**
"AGPL is prohibited for a USGS release" — we cannot support that. It is narrower
and, we think, airtight: **AGPL sits conspicuously far from the stated
CC0/public-domain preference, and we found nothing in the available guidance
affirming it as acceptable.** The combination of (a) the strongest
mainstream copyleft, (b) asserted private copyright, and (c) a paid-commercial
licensing track is about as distant from "unencumbered U.S. Government public-
domain work" as an inbound dependency can be. Whether that distance is
disqualifying is a release-authority / Solicitor determination, not ours — but
it is a strong reason to *reduce the exposure* rather than wait for a ruling
that may not exist in writing.

**Constructive breadcrumbs — how to shrink the AGPL exposure (for the
developers, should the authority want it reduced).** The good news from a
scoping pass this review ran is that the exposure is **more bounded than "the
whole Utilities core."** A current scan of the `Utilities` tree finds the
OpenHydroQual/AGPL banner in exactly the **`TimeSeries` family — `TimeSeries.{h,hpp}`
and `TimeSeriesSet.{h,hpp}` (4 files)**; the other `Utilities` sources SedSAT3
compiles (e.g. `Matrix*`, `Vector*`, `BTC*`, `Distribution*`, `QuickSort`,
`NormalDist`) did **not** carry the AGPL header in that scan. So the migration
target is the `TimeSeries`/`TimeSeriesSet` types specifically, not every file in
the submodule. Concrete, bounded pointers:

1. **Scope the real footprint first (cheap, do this before any rewrite).** Run a
   proper per-file license audit of `Utilities/` (the 4-file figure is a
   keyword-scan lower bound), then grep the SedSAT3 core for actual use of the
   AGPL-headered types. This review's rough pass shows `TimeSeries`/`TimeSeriesSet`
   are referenced across ~20 core translation units (`sourcesinkdata`,
   `conductor`, `concentrationset`, `MCMC`, the `cmb*` wrappers, `results`, and
   the charting layer), so this is a genuine refactor — but one aimed at a single
   type family with a well-defined interface, not an open-ended rewrite.
2. **Isolate behind the existing `cmb*` wrappers.** SedSAT3 already wraps these
   types in first-party classes (`cmbtimeseries`, `cmbtimeseriesset`,
   `cmbvector(set)(set)`). That indirection is an asset: it means a replacement
   time-series/container implementation can be swapped in behind those wrappers
   with the blast radius largely confined to the wrapper internals, rather than
   touching every call site.
3. **Replace the `TimeSeries` family with a permissive or first-party
   equivalent.** The used surface is modest (named series, append, indexed/named
   lookup, basic series math). Options, in rough order of least-residual-license-
   question: a clean first-party/federal re-implementation of just the needed
   container semantics (restores CC0 eligibility outright); or an existing
   permissively-licensed time-series/container type. Either removes the AGPL
   headers from the shipped product.
4. **While you are in there, the Test 7 crash fix lands in the same code.** The
   missing `append(const TimeSeries<T>&, const std::string&)` overload and the
   throwing name-lookup (§12a / Test 7) live in exactly these `TimeSeriesSet`
   files — a re-implementation is the natural place to add the intended overload
   and guard the lookups, so the license and the crash can be retired together.
5. **If re-implementation is not undertaken, get the grant in writing.** The
   lowest-engineering path remains option (1) in the list above — the author, as
   copyright holder, issuing USGS a permissive/CC0 grant for the specific
   SedSAT3-used `Utilities` files, captured on file with the release. That does
   not require touching code, but it does require the author's cooperation and a
   written instrument.

These are offered as *breadcrumbs*, not a mandate: the authority may decide the
AGPL distance is tolerable, or pursue a written grant instead of a rewrite. The
point is that if the goal is to move toward the CC0/public-domain preference, the
work is a contained, single-type-family migration — and it coincides with a
crash fix — rather than an unbounded one.

**Scope disclaimer (reviewer stance).** This finding is deliberately
**factual and non-conclusory**: it records the verified license headers, the
upstream dual-license terms, and the internal Apache-vs-AGPL inconsistency, and
routes the ownership/authority/licensing decision to the release authority. It
does not assert who owns the code or which resolution is correct. It should be
resolved **together with §13 (license) and §15 (authority)**, and — because path
(3) is a major engineering change and paths (1)/(2)/(4) have process/legal lead
time — raised **early**.

---

## 14. Hosting / release-metadata mismatch (code.usgs.gov vs. personal GitHub)

**What was verified.** The reviewed clone's `origin` is the USGS-official
release repository:

```
origin  https://code.usgs.gov/cphipps/SedimentSourceAssessmentTool3.git
```

(`cphipps` = Cara Peterman-Phipps, the third listed author.) So the software
**is** hosted on code.usgs.gov — an improvement over the earlier reading in the
Section 7 checklist. However, the project's own in-repo metadata still
advertises a **personal GitHub namespace** (`ArashMassoudieh/SedSat3`, the lead
author) as canonical:

- **`code.json`** (the machine-readable release descriptor USGS discoverability
  tooling consumes) declares GitHub for **every** URL: `homepageURL`,
  `repositoryURL`, `downloadURL`, `disclaimerURL`, and the license URL all point
  to `github.com/ArashMassoudieh/SedSat3`.
- **`README.md`** — clone command, `.deb` download, User Manual link, Issues
  link, and citation all reference the GitHub repo.
- **`createdeb.sh`** hardcodes `GITHUB_REPO="ArashMassoudieh/SedSat3"` and pulls
  releases from GitHub.
- **Submodules** point to GitHub (`ArashMassoudieh/Utilities`, `j2doll/QXlsx`).

**Interpretation.** The project was developed/released on personal GitHub and
later imported into the USGS track (consistent with the `import-sedsat-source`
branch and `usgs/main` merge in the history, Section 2). The migration did not
update the metadata, so the **official release artifact (`code.json`) directs
users and the USGS software catalog away from the code.usgs.gov home to a
personal GitHub account.** For a USGS release this is a metadata-correctness
issue: `code.json`'s `repositoryURL`/`homepageURL` should reflect the
code.usgs.gov location.

**Not verified here.** Whether `github.com/ArashMassoudieh/SedSat3` is currently
public, maintained, or intended to remain "upstream" was not checked (would
require querying GitHub directly). That answer decides the fix: if code.usgs.gov
is to be canonical, update all metadata/URLs to it; if GitHub remains the
development home with code.usgs.gov as the release mirror, document the
dual-hosting relationship explicitly. Either way the `code.json` a USGS release
publishes should resolve to the USGS-official repository.

**Recommendation.** Reconcile `code.json`, `README.md`, and `createdeb.sh` to
the canonical code.usgs.gov URL
(`https://code.usgs.gov/cphipps/SedimentSourceAssessmentTool3`), or clearly
document the GitHub/code.usgs.gov relationship. Related: the submodule-longevity
concern in Section 3 (dependencies on the author's personal GitHub) is the same
theme — a USGS-released product should not have its canonical identity or
critical dependencies rooted outside USGS control.

---

## 14a. Parallel public distribution (sedsat.org) and the shipped installer — verified

**What prompted this.** The repo `README.md` advertises a precompiled Windows
installer at `https://sedsat.org/download`. That site turns out to be a full,
live distribution channel, and inspecting the installer it serves surfaced
several facts that bear directly on §13, §13a, §14, and §15.

### 14a.1 The sedsat.org site (read 2026-10-15)

A live WordPress site (author archive `sedsat.org/author/massoudieh/`, posted
2023-04-03) with Download, Documentation, Open Source, Instructional Videos, and
Sample Data pages. Verified characteristics:

- Describes SedSAT3 as *"developed **in collaboration with** the U.S. Geological
  Survey"* — a collaboration framing, not a "USGS product" claim (relevant to
  §15).
- **Downloads are gated behind a lead-capture form** ("complete the short form
  below to access the download links"). An official USGS software product would
  not normally place its downloads behind a personal site's data-collection form.
- Gives a **third, different** set of Windows build instructions —
  **"Visual Studio 2022 + vcpkg"** — distinct from the repo README (generic
  qmake/CMake) and from this review's documented pixi/MSVC path. So a working
  Windows recipe exists, but it lives on the author's website, **not in the
  repo**.
- All code links point at the author's **personal GitHub** and
  `arashmassoudieh.github.io` (reinforces §14). The site's license statement is
  vague — *"refer to the LICENSE file in the repository"* — and does not surface
  the GPL/AGPL situation.

### 14a.2 The shipped installer `CMB_Source.msi` (inspected non-destructively)

The MSI served by the site was inspected by reading its installer database
(Property / File / Summary tables via the WindowsInstaller API — **no install,
no execution, no payload extraction**). Verified metadata:

| Field                 | Value (verbatim from the MSI)                                                     |
| --------------------- | --------------------------------------------------------------------------------- |
| ProductName           | **CMBSource** (not "SedSAT3")                                               |
| Manufacturer / Author | **EnviroInformatics**                                                       |
| ProductVersion        | **0.0.20** (not the repo's 1.1.6)                                           |
| Build tool            | **Advanced Installer 15.1** (matches the repo's tracked `CMB_Source.aip`) |
| Package type          | x64                                                                               |
| Prerequisites dir     | `[AppDataFolder]EnviroInformatics\CMBSource\prerequisites`                      |

Bundled runtime DLLs (from the File table):

- **`Qt5Core/Gui/Widgets/Charts/Svg.dll`, all v5.15.2.0** — i.e. the shipped
  binary is built against **Qt 5.15.2**, not Qt 6.
- **`gsl.dll` + `gslcblas.dll`** (GPL GSL), **`Qt5Charts.dll`** (GPL/commercial),
  plus `QXlsx.dll`, `blas_win64_MT.dll`, `lapack_win64_MT.dll`,
  `opengl32sw.dll`, `libEGL/libGLESv2.dll`, `D3Dcompiler_47.dll`, `qwindows.dll`.
- Application exe: **`CMBSource.exe`** (~1.2 MB), shipping `forms_structures.json`
  and `tools.json` (same resource files as the repo).

### 14a.3 What this establishes

1. **The public Windows binary is a different-lineage artifact than the reviewed
   source.** CMBSource **v0.0.20 / Qt 5.15.2 / EnviroInformatics** vs. the repo's
   SedSAT3 **v1.1.6 / Qt 6**. So the distributed installer was **not** produced
   from the source tree under review; it comes from the earlier "CMBSource"
   (Qt5) codebase and the author's own build/packaging setup. This resolves the
   apparent puzzle of "where does a Windows executable come from if the repo
   won't build on Windows?" — **it does not come from this repo.** The §6 finding
   (the current Qt6 source has no clean, documented Windows build) stands;
   a precompiled binary existing elsewhere does not contradict it.
2. **The §13 license exposure is realized in the shipped product, not just in
   source linkage.** The distributed installer actually **bundles and
   redistributes GPL `gsl.dll` and `Qt5Charts.dll`** as DLLs.
3. **The EnviroInformatics / CMBSource identity in the installer metadata**
   is concrete corroboration of §13a (EnviroInformatics-owned, commercially-
   licensed lineage) and §15 (authorship/authority): the public artifact is
   published under the author's company identity, in parallel with the USGS
   review track.

### 14a.4 Scope and limits (reviewer stance)

- The MSI facts come from its **database tables** (reliable declared metadata).
  The installer was **not** run, and its CAB payload was **not** extracted or
  executed — reported file names/versions are the installer's declared contents,
  not a runtime verification.
- The downloaded MSI is dated 2026-10-15 on the reviewer's machine; if the
  author later republishes a Qt6/1.1.6 installer, that artifact may differ from
  the one inspected here.
- "EnviroInformatics" / "CMBSource" / version / DLL set are verbatim/declared
  facts; the inference that this reflects a parallel EnviroInformatics-published
  product is strong but is recorded as an **observation**, routed (with §13a /
  §15) to the release authority — not a reviewer determination of ownership.

**Recommendation.** Treat the sedsat.org channel and the CMBSource/Qt5 installer
as part of the §13a/§14/§15 package for the release coordinator: establish what
the USGS release actually *is* (which source, which version, which framework
generation, which outbound license), and whether a personally-hosted,
EnviroInformatics-branded, GPL-DLL-bundling installer should be the thing a USGS
`README` points users to.

---

## 15. Authorship / release authority (exceeds code-review scope — escalate)

> *This section is the detailed treatment of the **Threshold question** at the
> top of this document (before §1). If you are arriving here from a cross-
> reference, that leading callout is the short version; the full discussion
> follows.*

> **This is not a code-review determination.** It is a provenance/authority
> question that a code reviewer can only *surface*, not resolve. It requires
> facts not available to this review and a decision by the USGS software-release
> authority (Center management / Science Publishing Network / Bureau Approving
> Official). It is recorded here because it is **antecedent to the license
> question (§13)** and, if unresolved, can block a release after significant
> downstream effort.
>
> **Concrete evidence now exists (see §13a):** the first-party `Utilities` core
> carries **AGPL-3.0 + commercial "OpenHydroQual"** headers asserting copyright by
> Arash Massoudieh / EnviroInformatics, LLC. This upgrades the authorship question
> below from "unclear" to "a core dependency is asserted **private, commercially-
> licensed IP**," and must be resolved jointly with §13 / §13a.

**Observed facts (from the repository; not judgments):**

- The **lead author (Arash Massoudieh) does not appear to be a USGS employee.**
  The software's canonical namespace, its `Utilities` submodule, and the QXlsx
  fork all live under his personal accounts; the general-purpose `Utilities`
  library is his cross-project personal code (Section 3b). *(This review did not
  independently verify his employment/affiliation status — the point is that the
  repository evidence points to non-USGS primary authorship, which needs
  confirmation.)*
- A **USGS employee (Cara Peterman-Phipps, `cphipps`)** holds the code.usgs.gov
  copy and is shepherding it through USGS review (this review was requested by
  her), indicating some USGS involvement/contribution.
- The product is **presented as USGS software**: `README.md` citation asserts
  "U.S. Geological Survey," and `code.json` + `DISCLAIMER.md` are present.
- **No public DOI-USGS (recognized USGS GitHub org) repository was found.**
  Note: DOI-USGS repos require an approval process to be made public, so one may
  exist privately — this could not be determined from the reviewed clone and
  should be checked.

**Why it matters (why it is antecedent to §13):**

USGS software release policy (SM 502.11 / FSPS) presumes the software is USGS
work — authored by federal employees, or produced under a contract / cooperative
agreement / grant that gives USGS the right to release it. If the substantive
authorship is non-federal, that basis is not automatic and must be established
via the appropriate instrument (agreement, contribution, or copyright
arrangement).

This directly entangles with the license blocker in **§13**: the CC0 /
public-domain default rests on federal authorship. Per the FSF's own FAQ, *"if
the program is written by US federal government employees in the course of their
employment, it is in the public domain… Since the GNU GPL is based on copyright,
such a program cannot be released under the GNU GPL."* The converse matters here:
if the primary author is **non-federal**, his contributions **are** copyrightable
by him, which changes (a) who holds copyright, (b) who has standing to choose the
outbound license, and (c) whether CC0 is even applicable to those parts. The
authorship question and the license question are therefore the **same question**
viewed from two angles, and §13 cannot be fully resolved until §15 is.

**Open questions for the release authority (not for the code reviewer):**

1. Is SedSAT3 a USGS software product, and under what authority (employment,
   cooperative agreement, grant, CESU, contract, or formal contribution)?
2. Is there a contribution / copyright-assignment / permission instrument on
   file covering the non-USGS author's code (cf. the "copyrighted material of
   others, used with permission, marked as such" spirit of SM 1100.6)?
3. Does a (possibly private, approval-pending) DOI-USGS repository exist, and is
   it intended as the recognized-org node (cf. the SWB2 dual-hosting model in
   `swb2/design/github_and_gitlab.md`, where org visibility routes through
   **DOI-USGS**, not a personal account)?

**Recommendation.** Escalate to Cara and the software-release coordinator /
Science Publishing Network before further release effort. Resolve §15 (authority)
and §13 (license) together, since the license decision depends on the authorship
determination. This finding is deliberately **non-conclusory**: it raises the
question and routes it to the owners; it does not assert whether SedSAT3 is or is
not a valid USGS product.

---

## 16. Spelling / naming hygiene (mechanically catchable)

Spelling errors are worth a systematic pass because they are mechanically
detectable at the source level and, in the case of identifiers and config keys,
carry real API/contract weight. A targeted scan surfaced three tiers, ordered by
severity/risk:

### 16a. `purturbation` / `purturb` (should be "perturbation") — spans all layers

- **Identifiers:** `purturbation_factor` (member, `MCMC.h:89`), `purturb(int)`
  (method, `MCMC.h:496`), plus `MCMC.hpp` usages.
- **Runtime settings-string keys:** `"initial_purturbation_factor"`,
  `"initial_purturbation"`, `"purturbation_change_scale"` parsed in `MCMC.hpp`.
- **User-facing labels:** `"Purturbation Factor"` as chart title / y-axis label
  (`conductor.cpp` ×4, `sourcesinkdata.cpp` ×2) — the misspelling is **visible
  to users on plots**.

**Contract-risk — verified low.** A concern was whether the misspelled settings
keys are persisted to the `.cmb` project file (which would make a rename a
backward-compatibility break). Checked directly against a saved project
(`examples/my_example_smw.cmb`): it contains **no `purturb` string**, and the
serialized `"Options"` block does not carry these keys. The project JSON is
built from `toJsonObject()` methods (`mainwindow.cpp:269–283`) that do not
include the perturbation keys. So `purturbation` is a **runtime/label**
misspelling, **not** a persistence contract — it can be corrected without
breaking saved projects. (The chart-label instances are pure display strings;
also free to fix.)

### 16b. `FileBrowserPushButtom` / `filebrowserpushbuttom` (should be "PushButton")

Class name, filenames, header guard, and all usages (`genericform.*`,
`filebrowserpushbuttom.{h,cpp}`). Purely internal API — no user-facing or
persistence contract — so a safe mechanical rename. (Also noted in §9 as a
naming-consistency issue; §16 confirms its full spread.)

### 16c. `"...target samle"` (should be "sample") — user-facing string

In `resources/forms_structures.json` (×2) — the tool description shown in the
GUI form for the Bayesian methods. Visible to users; trivially fixable; no
contract risk.

### Assessment and recommendation

None of these are correctness defects, but for a released USGS product the
user-facing instances (16a labels, 16c) are a professionalism/polish issue, and
the identifier instances (16a, 16b) are API hygiene. Because spelling is
mechanically catchable:

1. Fix the safe tiers now (16b rename, 16c string, 16a labels + identifiers —
   all verified free of persistence-contract risk).
2. **Add a spell-checker to the workflow.** `codespell` is the standard tool for
   source code (checks identifiers/comments/strings, low false-positive rate).
   Wire it as a pixi task alongside `lint`/`cppcheck` and run it in CI, turning
   "reviewer noticed some typos" into systematic prevention.
