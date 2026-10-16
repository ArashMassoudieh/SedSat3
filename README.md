# SedSAT3 — Sediment Source Assessment Tool, version 3

SedSAT3 is U.S. Geological Survey software for sediment source fingerprinting
and apportionment using chemical and isotopic tracers. It provides a graphical
user interface and a command line program with tools for preprocessing, tracer
selection, source apportionment and diagnostics.

Repository: <<https://code.usgs.gov/...final group path...>>

---

## Features

- **Graphical interface** built with Qt 6 for managing projects, configuring
  analyses and viewing results.
- **Command line program** (`sedsat3-cli`) that runs the same analyses from a
  JSON script, for batch work and reproducible runs.
- **Data import** from Microsoft Excel files with a standard sheet layout.
- **Preprocessing**: outlier detection, particle-size and organic-matter
  correction, bracketing analysis and discriminant function analysis (DFA).
- **Statistics**: ANOVA, automated tracer selection, tracer discriminant power,
  distribution fitting, correlation and covariance matrices, error analysis,
  Kolmogorov–Smirnov tests and Box–Cox transformations.
- **Source apportionment**:
  - deterministic: Levenberg–Marquardt maximum-likelihood fit
  - Bayesian: Markov chain Monte Carlo (MCMC) sampling, with a user-set
    random seed that is written to the output so any run can be repeated
- **Results** as tables and plots, including credible intervals, posterior
  distributions and predicted concentrations.

A genetic-algorithm (GA) solver is included in the source but disabled by
default, because Levenberg–Marquardt solves the same problem more efficiently
and offering two deterministic methods would confuse users (see
[Build options](#build-options)).

---

## User manual

The SedSAT3 user's manual is available at <<USGS manual URL/DOI>>.

The command line program has its own manual in
[`docs/manual/sedsat3-cli-manual.pdf`](docs/manual/sedsat3-cli-manual.pdf).

---

## Running the portable Windows build

The portable build needs no installer and no administrator rights.

1. Download `SedSAT3-<version>-windows-x64-portable.zip` from the repository's
   releases page.
2. Optionally check the download against the SHA-256 checksum published with it
   (`certutil -hashfile SedSAT3-<version>-windows-x64-portable.zip SHA256`).
3. Unzip it to any folder you can write to, for example your Desktop or
   `%LOCALAPPDATA%\SedSAT3`.
4. Double-click `SedSat3.exe`.

Keep the `resources` folder next to the executable; the program reads its tool
and form definitions from it. To remove SedSAT3, delete the folder.

---

## Build instructions

SedSAT3 builds on Windows, macOS and Linux.

### Prerequisites

- Qt 6.2 or later with the Charts and PrintSupport modules (Qt 6.8 is tested)
- GNU Scientific Library (GSL)
- Armadillo, with LAPACK and BLAS
- A C++17 compiler with OpenMP (GCC, Clang or MSVC)
- QXlsx, included as a git submodule and compiled with SedSAT3

### Get the source

```bash
git clone --recurse-submodules <<https://code.usgs.gov/...final group path...>>.git SedSAT3
cd SedSAT3
```

If you cloned without `--recurse-submodules`, run
`git submodule update --init --recursive`.

### Build with qmake (GUI and command line)

```bash
qmake6 SedSat3.pro        # use the qmake of your Qt 6 kit
make -j8                  # nmake or jom with MSVC

qmake6 SedSat3CLI.pro     # command line program
make -j8
```

### Build with CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
cmake --build build --parallel
ctest --test-dir build          # unit tests
```

### Build options

| qmake | CMake | Default | Effect |
|---|---|---|---|
| `CONFIG+=ga` | `-DENABLE_GA=ON` | off | builds the genetic-algorithm solver and its commands |
| — | `-DBUILD_TESTING=OFF` | on | skips the unit tests |

### Windows

See [`docs/BUILD-Windows-MinGW.md`](docs/BUILD-Windows-MinGW.md) for an MSYS2/MinGW
build and [`docs/PACKAGE-Windows-Portable.md`](docs/PACKAGE-Windows-Portable.md) for
producing the portable ZIP from an MSVC build.

### Tests

```bash
tests/regression/run.sh path/to/sedsat3-cli
```

runs the deterministic analyses on the projects in `tests/data/` and compares
them with `tests/baseline/`. See [`tests/README.md`](tests/README.md).

---

## Quick start

A sample dataset, `examples/SampleData.xlsx`, follows the standard layout: a
`Mixture` sheet with the target samples and one sheet per source (`Forest`,
`Crop`, `Pasture`, `Bank`). Each sheet has a `sample` column, particle-size
(`D50`) and organic-carbon (`TOC`) columns for the size and organic-matter
corrections, and one column per elemental or isotopic tracer. Every cell must
hold a finite number; empty cells, text, `NaN` and `Inf` are rejected at import
with the sheet, row and column of the cell.

1. Launch SedSAT3 and import the Excel file.
2. Choose the tracers to include (constituents with zero or negative values,
   such as δ¹³C, must be excluded or given the isotope role).
3. Apply preprocessing (outlier analysis, size and organic-matter correction).
4. Run an apportionment method and review the results.

---

## Disclaimer

This software is preliminary or provisional and is subject to revision. See
[DISCLAIMER.md](DISCLAIMER.md).

---

## License

See [LICENSE.md](LICENSE.md). Third-party components and their licenses are
listed in [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).

---

## Citation

<<Massoudieh, A., Peterman-Phipps, C., Gellis, A.C., … confirm full list>>,
<<year>>, SedSAT3—Sediment Source Assessment Tool, version 3 (version
<<version>>): U.S. Geological Survey software release,
https://doi.org/<<10.5066/XXXXXXX>>.

Machine-readable citation metadata is in [CITATION.cff](CITATION.cff).
