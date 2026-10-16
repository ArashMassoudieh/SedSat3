# Third-party licenses

SedSAT3's own source code is covered by [LICENSE.md](LICENSE.md). SedSAT3 is
built against, and its binary packages redistribute, the components below.
Each keeps its own license; nothing here relicenses them. The list was made
from the build files (`SedSat3.pro`, `SedSat3CLI.pro`, `CMakeLists.txt`,
`SedSat3.vcxproj`) and from the headers the source includes.

| Component | Version used | How SedSAT3 uses it | License | Where it comes from |
|---|---|---|---|---|
| Qt Core, Gui, Widgets, PrintSupport | 6.x (6.8.3 tested) | GUI framework; dynamically linked | LGPL-3.0 (Qt open-source license) | Qt installer or system packages; not in this repository |
| Qt Charts | 6.x | plotting in the result windows (`USE_QCHARTS`) | GPL-3.0 (Qt open-source license) | Qt installer or system packages; not in this repository |
| QCustomPlot | 2.1.1 | plotting widget, compiled into SedSAT3 | GPL-3.0 | vendored in `thirdparty/qcustomplot6/` (see `thirdparty/qcustomplot6/GPL.txt`) |
| QXlsx | 1.5.0 | reading Excel `.xlsx` files, compiled into SedSAT3 | MIT | git submodule `thirdparty/QXlsx` (upstream https://github.com/QtExcel/QXlsx; see `thirdparty/QXlsx/LICENSE`) |
| GNU Scientific Library (GSL) | 2.7 | distributions, CDFs, weighted linear regression | GPL-3.0 | system/vcpkg/MSYS2 package; not in this repository |
| Armadillo | 12.x | linear algebra | Apache-2.0 | system/vcpkg/MSYS2 package; not in this repository |
| LAPACK / BLAS (reference, OpenBLAS, or the `*_win64_MT` builds shipped with Armadillo) | — | used by Armadillo | BSD-3-Clause (reference LAPACK, OpenBLAS) | system/vcpkg package or the Armadillo Windows download |
| OpenMP runtime (`libgomp` with GCC, `libomp` with Clang, `vcomp140` with MSVC) | — | parallel MCMC chains | libgomp: GPL-3.0 with the GCC Runtime Library Exception; libomp: Apache-2.0 with LLVM exception; vcomp140: Microsoft Visual C++ redistributable terms | compiler toolchain |
| Microsoft Visual C++ runtime (Windows builds) | 14.x | C/C++ runtime | Microsoft Visual C++ redistributable terms | Visual Studio; copied app-local into the portable package |

## Notes for the release coordinator

- The GPL-3.0 components (Qt Charts, QCustomPlot, GSL) do not change the
  license of SedSAT3's own source code, but a **binary package** that links
  them is distributed under terms compatible with the GPL-3.0. Whether that is
  acceptable for a USGS release, or whether those components should be
  replaced, is the release authority's decision (USGS code review, Oct 2026,
  "GPL dependency chain").
- Qt's LGPL-3.0 modules are dynamically linked, so users can replace the Qt
  DLLs, as the LGPL requires.
- The icons in `resources/Icons/` and `resources/icons/` were made for the
  project. <<confirm: no third-party icon sets are included>>
