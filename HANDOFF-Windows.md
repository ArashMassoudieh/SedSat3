# HANDOFF: Windows round 2, after the domain review

Written 2026-10-22 on the Linux machine. For Arash and Claude Code on the
Windows development machine (Visual Studio 2022, pixi). The first round
(pixi results, MSVC `/W4`, cppcheck, the 1.1.8 portable package) is done;
its handoff is in git history (`git show 324f9db:HANDOFF-Windows.md`).

## Where things stand

- Branch **`review-fixes-oct2026`**, the source of GitLab merge request **!4**.
  GitHub `main` is at the same commit.
- Since the 1.1.8 package was built, the branch has the fixes for Lillian
  Gorman Sanisaca's domain review (`docs/review/response-domain-review/`):
  - crashes fixed: isotope with an excluded base element, discriminant
    power (multi-way), correlation matrix with the correction, OM/size
    regression on a small group;
  - D50 and TOC are never tested or selected as tracers;
  - δ¹³C can use TOC (the organic matter constituent) as its base element;
  - Error Analysis histogram uses all realizations;
  - the Tools menu is in workflow order, grouped;
  - chart fixes: palette, axis titles and ranges, p-value axes, legends;
  - step 4 shows discriminant scores and eigenvectors, and a leave-one-out
    confusion matrix (seven panels);
  - isotopes are not corrected for organic matter and size by default;
  - tool descriptions cite the manual.
- Bayesian fingerprinting (MCMC), changed after the domain review:
  - **convergence diagnostics**: the results end with two table panels,
    split R-hat and effective sample size per parameter, red when beyond
    1.01 / 400 (`include/mcmcdiagnostics.h`, GSL FFT); the batch writes them
    as `8_` and `9_` files;
  - **correlated proposal** (adaptive Metropolis, ported from
    OpenHydroQual's MCMC), **on by default**, with a "Correlated proposal"
    checkbox on the Bayesian forms (`include/MCMC/MCMC.h/.hpp`,
    `UpdateProposalCovariance`, `perturb`);
  - **Dissolve Chains off by default**;
  - defaults 100,000 samples / 20,000 burn-in;
  - the Bayesian batch skips target samples excluded in Include/Exclude
    Samples (it used to crash on them).
  For the example (CTAIL1, the 7 tracers of step 4, 8 chains), the manual's
  Table 4.1 compares the settings. At 400,000 samples, correlated, without
  dissolving, every parameter has R-hat ≤ 1.017 and ESS ≥ 428. At the default
  100,000 the contributions' R-hat is about 1.03–1.04, which is expected.
- Verified on Linux: 58 unit tests, the regression suite, and the charts
  rendered offscreen. **None of it has been compiled with MSVC or clicked
  through in the GUI.**
- The version stays **1.1.8**. The new package replaces the earlier 1.1.8
  zip, with a new SHA-256.

## 1. Update and build

```powershell
git fetch origin
git checkout review-fixes-oct2026
git pull
git submodule update --init --recursive
pixi run build
pixi run test
```

Expect the regression suite to match as before. The only known difference is
the Levenberg-Marquardt batch, up to about 7e-8 (round-off). The
`properties.cmb` baseline changed on this branch (D50/TOC left the
ANOVA/EDP/bracketing results), so compare against the committed baseline;
**do not** regenerate it on Windows.

If MSVC rejects something new, the likely places are:
- `generalchart.cpp`: `std::size` on a `QColor` array, `QMap`, `QLegendMarker`;
- `src/conductor.cpp`: `std::set`;
- `src/sourcesinkdata.cpp`: `#include <armadillo>`, `arma::pinv` in the
  confusion matrix, `std::round`;
- `include/mcmcdiagnostics.h`: `gsl/gsl_fft_complex.h`;
- `include/MCMC/MCMC.hpp`: `std::max<int>` and the new members.

Fix, rerun `pixi run test`, and commit.

Two unit tests are new for the sampler and take a few seconds each:
`mcmc_correlatedProposalReproducible` (same seed, 1, 2 and 3 threads: same
samples) and `mcmcDiagnostics`. If the first fails on MSVC, the correlated
proposal is not reproducible there. Report it: do not loosen the test.

`pixi run warnings` and `pixi run cppcheck` are optional this round. If you
run them, add the counts to Section 7 of the code review letter and to
`REVIEW_RESPONSE.md` as before.

## 2. Check the GUI by hand

`pixi run run-app`, import `examples/SampleData.xlsx` (target sheet:
Mixture), then:

1. **Constituent Properties:**
   - Set D50 to *Particle Size* and TOC to *Organic Carbon*.
   - Set N15 to *Isotope* with base N and ratio 0.003676, and C13 to
     *Isotope* with base TOC and ratio 0.011180.
   - The Include column toggles with one click, and "Include all elements"
     updates the table at once.
2. **Tools menu:** Deterministic, Bayesian, then Other Statistical Tools
   with four groups (Data exploration, Element selection, Discriminant
   function analysis, Uncertainty and verification).
3. **Step 2, Power, TOC and D50,** then Other Statistical Tools → Data
   exploration → Correct based on size and organic matter for CTAIL1: click
   the graph button (this used to crash).
4. **Step 4 with the correction for CTAIL1:**
   - seven panels, the last the confusion matrix (table button; rows the
     true groups, last column "% correct");
   - the scatter (panel 5) keeps each group's color when you change the
     pair, and starts on two different groups;
   - the p-value plot shows its y axis;
   - the x axis reads "Constituents, in the order added".
5. **Step 5, Fingerprinting, for CTAIL1, with and without the correction:**
   both run, and the isotope panel shows a modeled C13 (not empty).
6. **Elements' discriminant power (Multi-way), Error Analysis, Source
   Verification, Distribution Fitting:** each runs.
7. **Exclude N in Constituent Properties, run Fingerprinting:** a message
   about N15's base element, not a crash. Include N again.
8. **Bayesian step 5 for CTAIL1 with the defaults:**
   - the form shows "Correlated proposal" checked and "Dissolve Chains"
     unchecked;
   - the progress window closes at the end;
   - the MCMC samples plot shows whole sample numbers;
   - the last two panels are "Convergence: split R-hat" and "Convergence:
     effective sample size" (table button only), with values beyond 1.01 /
     400 in red. Most R-hat values are a little above 1.01 at 100,000
     samples; compare with the manual's Table 4.1;
   - running it again with the same random seed (shown in the results name)
     gives the same numbers.
9. **Bayesian batch:** exclude two target samples in Include/Exclude
   Samples (set them to No), run step 6 with 5,000 samples:
   - no folder for the excluded samples;
   - each folder has nine files, `8_Convergence_ split R-hat.txt` and
     `9_Convergence_ effective sample size.txt` included.

Note anything that looks wrong. Charts are rendered by Qt, so fonts may
differ from Linux, but the content should match the manual's figures.

## 3. Retake the screenshots

These manual figures still show the old program. Take them from this build,
with the example data set up as in step 2.1, and save them under the same
names in `docs/manual/SedSat_User's_Manual/Figures/`:

| File | What |
|---|---|
| `SelectSourceTargetGroups.png` | the target-sheet dialog after File → Import Data from Excel |
| `Constituent_Viz.png` | the main window with an element selected under Data (try C13: linear axis) |
| `MCMC_progress.png` | the progress window during Bayesian step 5 |
| `MCMC_folders.png` | Explorer: the per-target folders after the Bayesian batch |
| `MCMC_files.png` | Explorer: the nine files (and the samples file) in one of them |
| `95p_CI_table.png` | an excerpt of `3_Source Contribution Credible Intervals.txt` |
| `95p_CI_predicted.png` | an excerpt of `5_Predicted Samples Credible Intervals.txt` |

For the two folder screenshots, run the Bayesian batch with fewer samples
(for example 5,000) to keep it quick. Save the project first: the folders go
next to the project file.

Rebuilding the manual PDF needs LaTeX (MiKTeX worked for the letter): in
`docs/manual/SedSat_User's_Manual`, run `pdflatex main`, `bibtex main`, then
`pdflatex main` twice, and copy `main.pdf` to `SedSAT3_User_Manual.pdf`. If
that is inconvenient, commit the images and rebuild the PDF on Linux.

## 4. Rebuild the portable package (still 1.1.8)

Same procedure as last time: `docs/PACKAGE-Windows-Portable.md`, as adapted
for the pixi build in `REVIEW_RESPONSE.md` ("Windows portable package"), with
the 18 extra conda-forge DLLs. One change:

- **Sample data:** put `examples\SampleData.xlsx` in `sample-data\`, and do
  **not** include `virtual-mixture.xlsx`. Lilly's review found that its
  column names (`Calcium_µg/g`, `Nickelgg`, ...) differ from the example's.

Then the dependency walk, the restricted-token launch check, zip to
`SedSAT3-1.1.8-windows-x64-portable.zip`, and the SHA-256. Update the SHA-256
and size in `REVIEW_RESPONSE.md` ("Done on Windows, version 1.1.8") and in
Section 7 of the code review letter. Regenerate the letter's `.docx`:
`pixi run python docs/review/response-letter/tex2docx.py docs/review/response-letter/SedSAT3_code_review_response.tex docs/review/response-letter/SedSAT3_code_review_response.docx`.

Then upload the new zip to the GitLab package registry in place of the
earlier 1.1.8.

## 5. Commit and push

```powershell
git add -A
git commit -m "Windows round 2: MSVC build of the domain review fixes, screenshots, 1.1.8 package rebuilt"
git push origin review-fixes-oct2026:main      # GitHub
git push usgs review-fixes-oct2026             # GitLab, updates merge request !4
```

GitLab needs a personal access token with `write_repository` scope as the
password.

## 6. The emails (drafts in Gmail, not sent)

- **To Lilly** (cc Cara, Allen), "SedSAT3: response to your domain review, and
  the revised user's manual". Attach
  `docs/review/response-domain-review/SedSAT3_domain_review_response.docx`
  and `docs/manual/SedSat_User's_Manual/SedSAT3_User_Manual.pdf`. If you
  retook the screenshots, attach the rebuilt PDF. If the new package is
  uploaded, mention it.
- **To Allen** (cc Cara), asking for the Sed_SAT 1.0 code. Probably not
  needed: it is public at `https://code.usgs.gov/water/sed_sat`
  (DOI 10.5066/F76Q1VBX). Delete the draft or reword it.

## Do not

- Regenerate `tests/baseline/` on Windows. The baseline is the reviewed
  Linux output; Windows differences are something to report.
- Edit `thirdparty/`.
- Rename serialized keys in the forms or project files; old projects and
  scripts must keep loading.
- Raise the version: it stays 1.1.8.
