# SedSAT3 – Sediment Source Assessment Tool, version 3
# U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.
#
# The analysis code shared by the graphical program (SedSat3.pro), the command
# line program (SedSat3CLI.pro) and the unit tests (tests/unit/unittests.pro):
# sources, include paths, definitions, third-party libraries and compiler
# settings. Each project includes this file and adds its own sources.
#
# Options (add to the qmake command line):
#   CONFIG+=ga    build the genetic-algorithm solver and its commands. The GA
#                 works but is left out by default: Levenberg-Marquardt solves
#                 the same problem more efficiently, and offering two
#                 deterministic methods would confuse users.

SEDSAT3_ROOT = $$PWD

CONFIG += c++17

# A non-void function that falls off its end is undefined behaviour; make it a
# build error rather than a warning (USGS code review, Oct 2026)
gcc|clang: QMAKE_CXXFLAGS += -Werror=return-type
msvc: QMAKE_CXXFLAGS += /we4715

DEFINES += _arma SUPPORT_USE_QJSON GSL Q_GUI_SUPPORT
greaterThan(QT_MAJOR_VERSION, 5): DEFINES += Qt6

INCLUDEPATH += \
    $$SEDSAT3_ROOT \
    $$SEDSAT3_ROOT/include \
    $$SEDSAT3_ROOT/include/MCMC \
    $$SEDSAT3_ROOT/Utilities

SOURCES += \
    $$SEDSAT3_ROOT/Utilities/Distribution.cpp \
    $$SEDSAT3_ROOT/Utilities/Matrix.cpp \
    $$SEDSAT3_ROOT/Utilities/Matrix_arma.cpp \
    $$SEDSAT3_ROOT/Utilities/Matrix_arma_sp.cpp \
    $$SEDSAT3_ROOT/Utilities/QuickSort.cpp \
    $$SEDSAT3_ROOT/Utilities/Utilities.cpp \
    $$SEDSAT3_ROOT/Utilities/Vector.cpp \
    $$SEDSAT3_ROOT/Utilities/Vector_arma.cpp \
    $$SEDSAT3_ROOT/src/cmbdistribution.cpp \
    $$SEDSAT3_ROOT/src/concentrationset.cpp \
    $$SEDSAT3_ROOT/src/conductor.cpp \
    $$SEDSAT3_ROOT/src/elemental_profile.cpp \
    $$SEDSAT3_ROOT/src/elemental_profile_set.cpp \
    $$SEDSAT3_ROOT/src/interface.cpp \
    $$SEDSAT3_ROOT/src/numericparsing.cpp \
    $$SEDSAT3_ROOT/src/observation.cpp \
    $$SEDSAT3_ROOT/src/parameter.cpp \
    $$SEDSAT3_ROOT/src/sourcesinkdata.cpp \
    $$SEDSAT3_ROOT/cmbmatrix.cpp \
    $$SEDSAT3_ROOT/cmbtimeseries.cpp \
    $$SEDSAT3_ROOT/cmbtimeseriesset.cpp \
    $$SEDSAT3_ROOT/cmbvector.cpp \
    $$SEDSAT3_ROOT/cmbvectorset.cpp \
    $$SEDSAT3_ROOT/cmbvectorsetset.cpp \
    $$SEDSAT3_ROOT/contribution.cpp \
    $$SEDSAT3_ROOT/multiplelinearregression.cpp \
    $$SEDSAT3_ROOT/multiplelinearregressionset.cpp \
    $$SEDSAT3_ROOT/range.cpp \
    $$SEDSAT3_ROOT/rangeset.cpp \
    $$SEDSAT3_ROOT/resultitem.cpp \
    $$SEDSAT3_ROOT/results.cpp \
    $$SEDSAT3_ROOT/resultsetitem.cpp \
    $$SEDSAT3_ROOT/testmcmc.cpp

HEADERS += \
    $$SEDSAT3_ROOT/Utilities/Distribution.h \
    $$SEDSAT3_ROOT/Utilities/Matrix.h \
    $$SEDSAT3_ROOT/Utilities/Matrix_arma.h \
    $$SEDSAT3_ROOT/Utilities/Matrix_arma_sp.h \
    $$SEDSAT3_ROOT/Utilities/QuickSort.h \
    $$SEDSAT3_ROOT/Utilities/TimeSeries.h \
    $$SEDSAT3_ROOT/Utilities/TimeSeries.hpp \
    $$SEDSAT3_ROOT/Utilities/TimeSeriesSet.h \
    $$SEDSAT3_ROOT/Utilities/TimeSeriesSet.hpp \
    $$SEDSAT3_ROOT/Utilities/Utilities.h \
    $$SEDSAT3_ROOT/Utilities/Vector.h \
    $$SEDSAT3_ROOT/Utilities/Vector_arma.h \
    $$SEDSAT3_ROOT/include/analysishost.h \
    $$SEDSAT3_ROOT/include/cmbdistribution.h \
    $$SEDSAT3_ROOT/include/concentrationset.h \
    $$SEDSAT3_ROOT/include/conductor.h \
    $$SEDSAT3_ROOT/include/elemental_profile.h \
    $$SEDSAT3_ROOT/include/elemental_profile_set.h \
    $$SEDSAT3_ROOT/include/interface.h \
    $$SEDSAT3_ROOT/include/MCMC/MCMC.h \
    $$SEDSAT3_ROOT/include/MCMC/MCMC.hpp \
    $$SEDSAT3_ROOT/include/numericparsing.h \
    $$SEDSAT3_ROOT/include/observation.h \
    $$SEDSAT3_ROOT/include/parameter.h \
    $$SEDSAT3_ROOT/include/progressreporter.h \
    $$SEDSAT3_ROOT/include/randomstream.h \
    $$SEDSAT3_ROOT/include/sourcesinkdata.h \
    $$SEDSAT3_ROOT/cmbmatrix.h \
    $$SEDSAT3_ROOT/cmbtimeseries.h \
    $$SEDSAT3_ROOT/cmbtimeseriesset.h \
    $$SEDSAT3_ROOT/cmbvector.h \
    $$SEDSAT3_ROOT/cmbvectorset.h \
    $$SEDSAT3_ROOT/cmbvectorsetset.h \
    $$SEDSAT3_ROOT/contribution.h \
    $$SEDSAT3_ROOT/multiplelinearregression.h \
    $$SEDSAT3_ROOT/multiplelinearregressionset.h \
    $$SEDSAT3_ROOT/range.h \
    $$SEDSAT3_ROOT/rangeset.h \
    $$SEDSAT3_ROOT/resultitem.h \
    $$SEDSAT3_ROOT/results.h \
    $$SEDSAT3_ROOT/resultsetitem.h \
    $$SEDSAT3_ROOT/testmcmc.h

# Genetic algorithm (off by default; see the note at the top)
ga {
    DEFINES += SEDSAT3_ENABLE_GA
    INCLUDEPATH += $$SEDSAT3_ROOT/include/GA
    SOURCES += \
        $$SEDSAT3_ROOT/src/GA/Binary.cpp \
        $$SEDSAT3_ROOT/src/GA/GADistribution.cpp \
        $$SEDSAT3_ROOT/src/GA/Individual.cpp
    HEADERS += \
        $$SEDSAT3_ROOT/include/GA/Binary.h \
        $$SEDSAT3_ROOT/include/GA/GA.h \
        $$SEDSAT3_ROOT/include/GA/GA.hpp \
        $$SEDSAT3_ROOT/include/GA/GADistribution.h \
        $$SEDSAT3_ROOT/include/GA/Individual.h
}

# Third-party libraries
unix:!macx {
    LIBS += -larmadillo -llapack -lblas -lgsl -lpthread -lgomp
}

macx {
    # Homebrew prefix (Apple Silicon or Intel)
    exists(/opt/homebrew/include) {
        HOMEBREW_PREFIX = /opt/homebrew
    } else {
        HOMEBREW_PREFIX = /usr/local
    }

    INCLUDEPATH += $${HOMEBREW_PREFIX}/include
    INCLUDEPATH += $${HOMEBREW_PREFIX}/opt/libomp/include
    INCLUDEPATH += $${HOMEBREW_PREFIX}/opt/armadillo/include
    INCLUDEPATH += $${HOMEBREW_PREFIX}/opt/gsl/include
    INCLUDEPATH += $${HOMEBREW_PREFIX}/opt/openblas/include

    DEFINES += ARMA_DONT_USE_HDF5
    DEFINES += ARMA_DONT_USE_SUPERLU
    DEFINES += ARMA_DONT_USE_ARPACK

    QMAKE_MOC_OPTIONS += -DARMA_DONT_USE_HDF5
    QMAKE_MOC_OPTIONS += -DARMA_DONT_USE_SUPERLU
    QMAKE_MOC_OPTIONS += -DARMA_DONT_USE_ARPACK

    LIBS += -L$${HOMEBREW_PREFIX}/lib
    LIBS += -L$${HOMEBREW_PREFIX}/opt/libomp/lib
    LIBS += -larmadillo -lgsl -lgslcblas -llapack -lblas -lpthread -lomp

    # OpenMP on Apple clang requires explicit flags
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    QMAKE_LFLAGS   += -lomp
}

# Windows with MSVC, using the same dependency layout as SedSat3.vcxproj:
# armadillo and vcpkg checked out next to this repository.
win32-msvc {
    DEPS_ROOT = $$clean_path($$SEDSAT3_ROOT/..)

    INCLUDEPATH += $${DEPS_ROOT}/armadillo/armadillo-header
    INCLUDEPATH += $${DEPS_ROOT}/vcpkg/installed/x64-windows/include

    DEFINES += NOMINMAX ARMA_USE_LAPACK ARMA_USE_BLAS

    LIBS += -L$${DEPS_ROOT}/armadillo/lapack-blas_lib_win64
    LIBS += -L$${DEPS_ROOT}/vcpkg/installed/x64-windows/lib
    LIBS += -lblas_win64_MT -llapack_win64_MT -lgsl -lgslcblas

    QMAKE_CXXFLAGS += /Zc:__cplusplus

    # The bundled armadillo has no SuperLU, so spsolve() is unavailable. The
    # sparse matrix class is not used by any analysis, and SedSat3.vcxproj
    # leaves it out for the same reason.
    SOURCES -= $$SEDSAT3_ROOT/Utilities/Matrix_arma_sp.cpp
}

# OpenMP
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_OPENMP
QMAKE_LFLAGS   += $$QMAKE_LFLAGS_OPENMP

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}
