# SedSAT3 (Sediment Source Assessment Tool, version 1.0.25)

SedSAT3 is an open-source software package developed by the U.S. Geological Survey for sediment source fingerprinting using chemical and isotopic tracers. It provides a graphical user interface and a suite of tools for preprocessing, tracer selection, source apportionment, and diagnostic analysis. The software supports multiple apportionment approaches, including deterministic maximum likelihood estimation, metaheuristic search with genetic algorithms, and Bayesian inference through Markov Chain Monte Carlo (MCMC) sampling.

---

## Features

- **Cross-platform GUI** built with Qt6 for managing projects, configuring simulations, and visualizing results.  
- **Data import** from Microsoft Excel files with a standardized tab structure.  
- **Preprocessing tools** including outlier detection, particle size and organic matter corrections, bracketing analysis, and discriminant function analysis (DFA).  
- **Statistical routines** such as ANOVA, automated tracer selection, tracer discriminant power evaluation, distribution fitting, correlation matrices, t-tests, error analysis, Kolmogorov–Smirnov tests, and Box–Cox transformations.  
- **Source apportionment methods**:
  - Deterministic maximum likelihood (Levenberg–Marquardt optimization)  
  - Genetic algorithms for global search  
  - Bayesian inference via MCMC sampling  
- **Results output** with visual and tabular summaries, including credible intervals, posterior distributions, predicted concentrations, and diagnostic plots.  

---

## Installation

### Windows
A precompiled installer is available at:  
👉 https://sedsat.org/download  

The installer bundles all required libraries, so no extra setup is needed.  

### Linux (Ubuntu/Debian)

You can install SedSAT3 from the prebuilt `.deb` package:

```bash
wget https://github.com/ArashMassoudieh/SedSat3/releases/download/Latest/SedSat3-Linux.deb
sudo apt install ./SedSat3-Linux.deb

Alternatively, you can build from source (see below).  

### macOS
A `.dmg` package will be provided in future releases. Until then, build from source using CMake or qmake.  

---

## Build from Source

SedSAT3 can be compiled on Windows, macOS, and Linux. It depends on several third-party libraries and a Git submodule (**QXlsx**).  

### Prerequisites

- **Qt 6** (Core, Gui, Widgets, PrintSupport, Charts)  
- **GNU Scientific Library (GSL)**  
- **Armadillo** (with BLAS/LAPACK support)  
- **QXlsx** (bundled as a submodule in `thirdparty/QXlsx`)  
- **C++17** or newer compiler (MSVC, g++, or clang)  

---

### Clone the Repository

```bash
git clone --recursive https://github.com/ArashMassoudieh/SedSat3.git
cd SedSat3
```

If you cloned without `--recursive`, fetch submodules manually:

```bash
git submodule update --init --recursive
```

---

### Build with **CMake** (recommended)

#### Linux (Ubuntu/Debian)

Install prerequisites:

```bash
sudo apt update
sudo apt install -y     build-essential cmake git     qt6-base-dev qt6-base-dev-tools qt6-charts-dev     libarmadillo-dev libblas-dev liblapack-dev     libgsl-dev
```

Build:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --parallel
```

The executable will be in:

```
build/bin/SedSat3
```

#### macOS (Homebrew)

```bash
brew install cmake qt armadillo gsl
mkdir -p build
cd build
cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt)/lib/cmake ..
cmake --build . --parallel
```

#### Windows (MSVC)

```powershell
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2019_64\lib\cmake"
cmake --build build --config Release
```

Executable will be at:

```
build\bin\Release\SedSat3.exe
```

---

### Build with **qmake** (developer option)

If you already have Qt installed with `qmake`:

```bash
qmake SedSat3.pro
make        # or `nmake` on Windows
```

---

## Quickstart Example

1. Prepare your source and target sediment data in Excel format (see `examples/`).  
2. Launch SedSat3.  
3. Import your Excel file.  
4. Apply preprocessing (outlier detection, tracer selection, corrections).  
5. Choose an apportionment method (MLE, GA, MCMC) and run analysis.  
6. View source contribution estimates, posterior distributions, and diagnostic plots.  

---

## Documentation

- **User Manual**: Detailed descriptions of preprocessing routines, tracer selection methods, and apportionment algorithms are provided in the SedSAT3 User’s Manual:  
https://sedsat.org/docs  

---

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests. Issues can be reported through the GitHub Issues tab:  
https://github.com/ArashMassoudieh/SedSat3/issues  

---

## License

SedSAT3 is distributed under the GNU General Public License v3.0 (GPL-3.0).

---

## Citation

If you use SedSAT3 in your research, please cite:

> Massoudieh, A., et al. (2023). *SedSAT user's manual. (https://github.com/ArashMassoudieh/SedSat-User-s-Manual/blob/main/SedSat_User_s_Manual.pdf)

---

## Acknowledgments

SedSAT3 builds on several open-source libraries, including Qt, GSL, Armadillo, and QXlsx. The project also uses the `Utilities` submodule developed and maintained separately.
