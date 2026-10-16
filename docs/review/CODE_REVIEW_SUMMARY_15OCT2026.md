# SedSAT3 — Code Review Summary

**Software:** SedSAT3 v1.1.6 — C++17 / Qt6 desktop app for sediment source
fingerprinting and apportionment.
**Repository reviewed:** `code.usgs.gov/cphipps/SedimentSourceAssessmentTool3`
(`main`, commit `ce7bc1d`).
**Review type:** USGS software code review (build, code, docs, licenses,
metadata, static analysis, GUI testing). Scientific validity (domain review) and
security review are separate and out of scope.

**Verdict.** In terms of a code review, the software is in reasonable shape: it builds, runs
its core methods, and its in-code documentation is a real strength. The code
defects are bounded and fixable — none are architecturally deep.  It could use some unit

testing, simple bugfixes, attention to compiler-generated static build artifacts, and cleanup

(spell checking and the like).

What could actually block a release are the **license** and **authorship** questions, which
are institutional decisions, not code fixes.

> *Note from Stephen Westenbroek:* I believe the bulk of this material is valid.
> Kiro (the AI assistant that produced most of it) tends toward verbosity and
> confident-sounding pronouncements; I pushed throughout to keep claims tied to
> linting, static analysis, or my own manual testing. Apologies in advance for
> any cringe-worthy phrasing it slipped in. Firm claims are backed by tool output
> or testing; softer ones are flagged — when it matters, trust the cited code or
> tool output over the prose.

---

## Threshold question (resolve first): is this a USGS product?

It is not clear from the reviewed materials that this codebase is, in substance,
a USGS product — and this is antecedent to everything else. The *original*
Sed_SAT (R + MS Access) was a released USGS tool (IPDS IP-079331, OFR 2017-1062).
The code reviewed here is a **new, independently-authored C++/Qt reimplementation**
of that tool's methodology, and its license and authorship point outside USGS:
AGPL/EnviroInformatics headers in the core, personal-namespace hosting, and a
separately-distributed "CMBSource" installer. This does not mean it *cannot* be a
USGS release — a non-federal author's work can be released under the right
instrument — but that basis is not visible, and the evidence points the other
way. This belongs with the release coordinator before further effort.

**[Westenbroek note: this is a legitimate concern and should be figured out. An official USGS code should not be pointing to personal repos. That said, Arash could easily continue development of the code in his personal github and submit pull requests against the 'official' copy on code.usgs.gov. ]**

---

## The three things that can block release (institutional — escalate)

These are entangled; resolve them together with the release coordinator (and,
given asserted private copyright, likely the Office of the Solicitor).

**1. First-party AGPL / commercial code in the core.** The `Utilities` core
(the `TimeSeries`/`TimeSeriesSet` family) carries AGPL-3.0 + "purchase a
commercial license" headers copyrighted by Arash Massoudieh / EnviroInformatics,
LLC — contradicting the submodule's own Apache-2.0 `LICENSE.md`. AGPL is the
strongest copyleft and the furthest from the USGS CC0 default. We found no USGS
guidance affirming *or* forbidding AGPL, so the finding is "conspicuously far
from the preference," not "prohibited."
*Fix:* the exposure is bounded to ~4 `TimeSeries*` files behind the existing
`cmb*` wrappers — either re-implement that one type family (which also fixes the
crash in B1), or get a written CC0/permissive grant from the author for the
SedSAT3-used files.

**2. GPL dependency chain.** The repo is GPL-3.0, outside the USGS preference
(CC0/MIT/Unlicense), because it links three GPL components: **GSL**,
**qcustomplot6**, and **Qt Charts**. (Qt *core* is LGPLv3 and is **not** a
blocker — the GUI need not be removed.)
*Fix:* move the plotting layer to a permissive library (e.g. QWT) or self-draw on
LGPL Qt core, and replace the single `gsl_multifit_wlinear` call with Armadillo
(already a dependency) or Eigen — or obtain a documented policy exception.

**3. Authorship / release authority.** The canonical identity, the `Utilities`
submodule, and the QXlsx fork all live under the author's personal GitHub, which
points toward non-USGS primary authorship (unverified — needs confirmation). The
CC0 default rests on federal authorship, so this is antecedent to the license
decision. Raise it; we are not asserting the answer.

> *Licensing-guidance gap (open).* How the USGS preference applies to real cases
> — federal code vendoring a permissive module, and the copyleft (GPL/LGPL/AGPL)
> case — has been raised on the CDI-Software Teams channel and is unanswered.
> FAQ #137 covers only the clean 100%-federal case and routes everything else to
> the Solicitor; a follow-up produced a suggested contact but no confirmed owner.
> Until resolved, the acceptable-license set and copyleft treatment are the
> release authority's to confirm.

**[Westenbroek note: Kiro likes complicated engineering solutions. A simpler solution might be to do something like I did with my SWB2 code. see: [github.com/smwesten-usgs/swb2/blob/main/THIRD_PARTY_LICENSES.md](https://github.com/smwesten-usgs/swb2/blob/main/THIRD_PARTY_LICENSES.md). Jonathan King released his 'pfdf' code after deliberately considering the licensing requirements of the packages his code uses. I'd pursue this before reengineering the whole package... ]**

---

## Concrete code defects (fix these)

- **Distribution Fitting crashes the app** (highest-confidence finding;
  debugger-confirmed). A `TimeSeriesSet::append(series, name)` overload doesn't
  exist, so fit results are stored unnamed; the later name lookup throws
  unhandled (`conductor.cpp:1588–1590` → `TimeSeriesSet.hpp:302`). *Fix:* add the
  intended overload, guard the lookups (`Contains()` exists but is unused), and
  surface a message instead of throwing.
- **"Include all elements" checkbox is inverted** — checking it *excludes* all
  tracers (`formelementinformation.cpp:28–36`). One-line argument swap.
- **Missing `return` in both `CheckNegativeElements` overloads**
  (`conductor.cpp:213` and the `map<...>` form) — the live input guard falls off
  the end on its clean path (undefined behavior, currently masked). Add
  `return true;`.
- **Other static-analysis errors (cppcheck):** null deref in
  `CBinary::operator[]` (`src/GA/Binary.cpp:139`); writes to a closed `FILE*`
  (`src/GA/GA.cpp:308–309`); possible `log()` of a non-positive value in
  `ApplyBoxCoxTransform` (`concentrationset.cpp:386`, worth confirming). Adopt the
  now-wired `lint`/`cppcheck`/`check` tasks in CI.

**[Westenbroek note: these comments are all legitimate. I have pages of supporting information if you want it. However, the noise to signal ratio is high. Let me know if you want more detail!]**

---

## Should address (not blocking)

- **Dead QXlsx submodule URL** — `.gitmodules` points at a repo that returns "not
  found," so a fresh clone fails for everyone. Repoint to the QtExcel upstream.
  Quick, high-value.
- **No automated tests / CI** beyond a docs build. Add at least one end-to-end
  regression against a known result, and run the static-analysis tasks in CI.
- **MCMC is not reproducible** — clock-seeded RNG, no seed control, no recorded
  seed (the deterministic Levenberg–Marquardt path reproduces exactly, confirming
  it's RNG-specific). The global `rand()` is also used unsafely inside an OpenMP
  region. Route stochastic methods through a single seedable, thread-safe PRNG
  and record the seed.
- **Metadata points at personal GitHub, not code.usgs.gov** — `code.json`,
  README, `createdeb.sh`, and the submodules. Also: the README's Windows-installer
  link (sedsat.org) serves a different-lineage artifact — "CMBSource" by
  "EnviroInformatics," v0.0.20, Qt 5.15.2, bundling GPL DLLs — i.e. the public
  binary was not built from this repo. Reconcile metadata and raise the parallel
  distribution with the coordinator.
- **Genetic algorithm advertised but not built** — the README/`code.json` claim a
  GA, but it's in no menu and doesn't compile (missing `StringOP.h`). Either wire
  it up or drop the claim. Note `MCMC.h` includes `GA.h`, so the GA header isn't
  cleanly deletable.

---

## Optional / polish

- **Repo layout mid-transition.** Non-GUI core still sits at the repo root; a few
  orphan files. Builds fine, but navigability suffers. Qt's own guidance puts
  both GUI and core under `src/<target>/` with a thin root `CMakeLists.txt` —
  worth adopting for a long-term USGS-maintained product.
- **README is truncated** (opening sentence cut off mid-word) and reads as
  adapted from the predecessor "CMBSource" project — out of sync with the current
  source and the shipped installer. Worth one deliberate reconciliation pass.
- **Spelling** in a few identifiers/labels (`purturbation`,
  `FileBrowserPushButtom`, "samle") — mechanically fixable; consider `codespell`
  in CI.
- **Attribution:** no per-file license headers; no shipped dependency-license
  (NOTICE) inventory.
- Minor GUI friction (label-flipping checkbox; multi-click deselection).
- Three coexisting build systems with no documented source of truth.

---

## Alignment with USGS release expectations (brief)

The mechanical file-presence items (README / DISCLAIMER / LICENSE / code.json)
are present. The substantive gaps are the **license content** (above),
**tests/reproducibility**, and the **process/authority** items (authorship, the
separate domain and security reviews, and any center-level procedures). The
`code.json` "Development" status reads as a Software Project posture, so DOI /
immutable-tag obligations don't yet apply. Guidance is layered and evolving;
confirm the governing version with your release coordinator.

---

## Verified vs. handed off

- **Verified:** build/launch (Windows + pixi); static analysis; MCMC
  non-reproducibility; the Distribution Fitting crash; the inverted checkbox and
  missing returns; the dead submodule URL; the dependency and `Utilities` license
  facts.
- **Domain review:** scientific validity of methods/priors/corrections; magnitude
  of MCMC run-to-run variation; whether OM/size correction yielding negatives is
  acceptable.
- **Release authority:** the license decision and the authorship question.
- **Not done (optional):** NaN downstream-effect test; some lower-priority test
  items; the security review.

Full detail and evidence: `CODE_REVIEW_OBSERVATIONS.md` (§1–16),
`TEST_RESULTS.md`, `BUILDING_ON_WINDOWS_WITH_PIXI.md`.
