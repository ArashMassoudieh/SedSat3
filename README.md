# SedSAT3 (Sediment Source Assessment Tool, version 1.0.25)

SedSAT3 is an open-source software package developed by the U.S. Geological Survey for sediment source fingerprinting using chemical and isotopic tracers. It provides a graphical user interface and a suite of tools for preprocessing, tracer selection, source apportionment, and diagnostic analysis. The software supports multiple apportionment approaches, including deterministic maximum likelihood estimation, metaheuristic search with genetic algorithms, and Bayesian inference through Markov Chain Monte Ca...

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
https://sedsat.org/download

### Linux (Ubuntu/Debian)

You can install SedSAT3 from the prebuilt `.deb` package:

```bash
wget https://github.com/ArashMassoudieh/SedSat3/releases/download/Latest/SedSat3-Linux.deb
sudo apt install ./SedSat3-Linux.deb
```

This installs SedSAT3 to `/usr/local/sedsat3` with:

- Executable at `/usr/local/sedsat3/bin/SedSat3`
- Resources at `/usr/local/sedsat3/resources`
- A symlink `/usr/local/bin/sedsat3` so you can run the app by typing `sedsat3`
- A desktop launcher at `/usr/share/applications/sedsat3.desktop`, so the app appears in your GNOME/KDE menu with its icon.

### macOS
Build from source (instructions below).

---

## Build from Source

SedSAT3 can be compiled on Windows, macOS, and Linux. The software depends on several third-party libraries and a Git submodule.

### Prerequisites

- Qt 6 (including `qmake` or CMake)  
- GNU Scientific Library (GSL)  
- Armadillo  
- QXlsx (included as a submodule)  
- A C++17 or newer compiler (MSVC, g++, or clang)  

### Clone the Repository

```bash
git clone https://github.com/ArashMassoudieh/SedSat3.git
cd SedSat3
```

Initialize and update the `Utilities` submodule:

```bash
git submodule init
git submodule update
```

### Build Instructions

#### Using qmake (quick testing)

```bash
qmake SedSat3.pro
make        # or `nmake` on Windows
```

#### Using CMake (for packaging and distribution)

```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel
cpack -G DEB   # to generate a Debian package
```

The `.deb` file will appear in the `build/` folder.

---

## Running SedSAT3

After installation, you can start SedSAT3 in several ways:

- From the terminal:
  ```bash
  sedsat3
  ```
- From the applications menu (look for "SedSat3" with its icon).  
- Or directly:
  ```bash
  /usr/local/sedsat3/bin/SedSat3
  ```

---

## Quickstart Example

1. Prepare your source and target sediment data in Excel format, following the standard tab layout (see sample files in the `examples/` folder).  
2. Launch SedSAT3 and create a new project.  
3. Import your Excel file through the GUI.  
4. Apply preprocessing (e.g., tracer selection, corrections).  
5. Configure an apportionment method (e.g., MCMC) and run the analysis.  
6. View outputs such as posterior distributions, source contribution estimates, and diagnostic plots.  

---

## Documentation

- **User Manual**: Detailed descriptions of preprocessing routines, tracer selection methods, and apportionment algorithms are provided in the SedSAT3 User’s Manual:(https://github.com/ArashMassoudieh/SedSat-User-s-Manual/blob/main/SedSat_User_s_Manual.pdf)  

---

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests. Issues can be reported through the GitHub Issues tab: https://github.com/ArashMassoudieh/SedSat3/issues

---

## License

SedSAT3 is distributed under the GNU General Public License v3.0 (GPL-3.0).

---

## Citation

If you use SedSAT3 in your research, please cite:

Massoudieh, A., et al. (2023). *SedSAT3: An open-source software for sediment source apportionment*. U.S. Geological Survey. https://github.com/ArashMassoudieh/SedSat3

---

## Acknowledgments

SedSAT3 builds on several open-source libraries, including Qt, GSL, Armadillo, and QXlsx. The project also uses the `Utilities` submodule developed and maintained separately.
