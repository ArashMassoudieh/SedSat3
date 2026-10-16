# SedSAT3 tests

## Regression tests (`tests/regression/`)

`run.sh` runs the deterministic analyses — project load, Box-Cox, outlier
analysis, Kolmogorov–Smirnov, distribution fitting and Levenberg–Marquardt
(batch) — on the projects in `tests/data/` through the command line build,
and compares each report against `tests/baseline/` with `compare.py`.

```
tests/regression/run.sh path/to/sedsat3-cli
```

Numbers are compared with a relative tolerance of 1e-9 (absolute floor 1e-12).
Timing fields are ignored. The MCMC is not part of this comparison because it
is stochastic; its reproducibility is covered by the unit tests.

The baseline was recorded on the unmodified code (commit `bde1341`) and is
only regenerated (`UPDATE_BASELINE=1 tests/regression/run.sh ...`) when a bug
fix explains every change; `docs/review/REVIEW_RESPONSE.md` lists those.

## Data (`tests/data/`)

- `Isotope_test.cmb` — five groups (Bank, Crop, Forest, Pasture, Mixture),
  24 constituents including the isotopes C13 and N15. Targets CTAIL2–CTAIL5
  are the mean profiles of individual sources, so the unmixing has a known
  answer.
- `properties.cmb` — the project used in the USGS domain review.
