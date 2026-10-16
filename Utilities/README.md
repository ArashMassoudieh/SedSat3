# Utilities

Numerical support classes used by SedSAT3: vectors and matrices (with
Armadillo-backed variants), the `TimeSeries`/`TimeSeriesSet` containers,
probability distributions, sorting and string helpers.

These files were written by Arash Massoudieh. Until October 2026 they were a
git submodule pointing at the author's separate `Utilities` library; they were
brought into this repository (from that library's commit `5db8d14`) during the
USGS code review so that the repository is self-contained and does not depend
on a personal repository. Only the files SedSAT3 builds are included, and they
are now maintained here as part of SedSAT3 under the repository's
[LICENSE.md](../LICENSE.md).
