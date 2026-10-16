# SedSAT3 tests

## Unit tests (`tests/unit/`)

Qt Test cases for the defects found in the USGS code review (October 2026)
and for the reproducibility of the MCMC: the numeric parser, the Box–Cox
transform, `CBinary::operator[]`, `Conductor::CheckNegativeElements`, the
"Include all elements" check box, distribution fitting on empty and short
data, the random-number streams, and the MCMC with a fixed seed (same seed →
same samples, different seed → different samples, same samples with 1, 3 or 8
threads).

```
qmake6 tests/unit/unittests.pro && make && ./sedsat3-unittests     # qmake
ctest --test-dir build -R unit                                      # CMake
```

They need no display: the program selects Qt's offscreen platform.

## Regression tests (`tests/regression/`)

`run.sh` runs the deterministic analyses — project load, Box-Cox, outlier
analysis, Kolmogorov–Smirnov, distribution fitting, ANOVA, correlation and
covariance matrices, bracketing, DFA, EDP and Levenberg–Marquardt (batch) — on
the projects in `tests/data/` through the command line build, and compares
each report with `tests/baseline/` using `compare.py`.

```
tests/regression/run.sh path/to/sedsat3-cli [output-dir]
ctest --test-dir build -R regression                                # CMake
```

**Tolerance:** numbers must agree to a relative 1e-9 (absolute floor 1e-12).
That absorbs differences in the last few bits between compilers and BLAS
libraries but catches any change in method. Timing fields and file paths are
not compared. The MCMC is stochastic and is not part of this comparison; its
reproducibility is covered by the unit tests.

### Baselines

- `tests/baseline/*_report.json` — the expected output of the current code.
  CI fails if a change moves any value beyond the tolerance. Regenerate with
  `UPDATE_BASELINE=1 tests/regression/run.sh path/to/sedsat3-cli` only when a
  deliberate fix explains every difference, and record the explanation in
  `docs/review/REVIEW_RESPONSE.md`.
- `tests/baseline/bde1341/` — the same scripts run on the code as it was
  before the October 2026 review fixes (commit `bde1341`), kept so that every
  change the fixes made can be traced. `docs/review/REVIEW_RESPONSE.md`
  explains each difference between the two.

## Data (`tests/data/`)

- `Isotope_test.cmb` — five groups (Bank, Crop, Forest, Pasture, Mixture),
  24 constituents including the isotopes C13 and N15. Targets CTAIL2–CTAIL5
  are the mean profiles of individual sources, so the unmixing has a known
  answer.
- `properties.cmb` — the project used in the USGS domain review: the same
  source samples, with the target group named "Target".
