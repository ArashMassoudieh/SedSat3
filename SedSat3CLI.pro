# Command line build of SedSAT3.
#
# Runs the same analyses as the graphical application by driving the same
# Conductor, but reports through a console host instead of windows. It shares
# the existing sources rather than a separate library: the list below is the
# analysis half of SedSat3.pro, with every file belonging to the interface left
# out.
#
#   qmake SedSat3CLI.pro && make
#   ./sedsat3-cli script.json
#
# QtWidgets is still linked, because Interface::ToTable() returns a
# QTableWidget and Interface is a base of every result type. No widget is ever
# created, so the program runs with QCoreApplication and needs no display.

QT += core gui widgets
CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = sedsat3-cli
TEMPLATE = app

DEFINES += _arma SUPPORT_USE_QJSON GSL Q_GUI_SUPPORT Qt6

QT_VERSION = $$[QT_VERSION]
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore/$${QT_VERSION}/QtCore/private
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtGui/$${QT_VERSION}/QtGui/private

INCLUDEPATH += \
    . \
    cli \
    include \
    include/GA \
    include/MCMC \
    Utilities

SOURCES += \
    cli/main_cli.cpp \
    cli/consolehost.cpp \
    cli/scriptrunner.cpp \
    Utilities/Distribution.cpp \
    Utilities/Matrix.cpp \
    Utilities/Matrix_arma.cpp \
    Utilities/Matrix_arma_sp.cpp \
    Utilities/QuickSort.cpp \
    Utilities/Vector.cpp \
    Utilities/Vector_arma.cpp \
    Utilities/Utilities.cpp \
    src/GA/Binary.cpp \
    src/GA/GADistribution.cpp \
    src/GA/Individual.cpp \
    src/cmbdistribution.cpp \
    src/concentrationset.cpp \
    src/conductor.cpp \
    src/elemental_profile.cpp \
    src/elemental_profile_set.cpp \
    src/interface.cpp \
    src/observation.cpp \
    src/parameter.cpp \
    src/sourcesinkdata.cpp \
    cmbmatrix.cpp \
    cmbtimeseries.cpp \
    cmbtimeseriesset.cpp \
    cmbvector.cpp \
    cmbvectorset.cpp \
    cmbvectorsetset.cpp \
    contribution.cpp \
    multiplelinearregression.cpp \
    multiplelinearregressionset.cpp \
    range.cpp \
    rangeset.cpp \
    resultitem.cpp \
    results.cpp \
    resultsetitem.cpp \
    testmcmc.cpp

HEADERS += \
    cli/consolehost.h \
    cli/scriptrunner.h \
    include/analysishost.h \
    include/progressreporter.h \
    include/conductor.h \
    include/sourcesinkdata.h

unix:!macx {
    LIBS += -larmadillo -llapack -lblas -lgsl -lpthread -lgomp
}

macx {
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

    LIBS += -L$${HOMEBREW_PREFIX}/lib
    LIBS += -L$${HOMEBREW_PREFIX}/opt/libomp/lib
    LIBS += -larmadillo -lgsl -lgslcblas -llapack -lblas -lpthread -lomp

    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
    QMAKE_LFLAGS   += -lomp
}

QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_OPENMP
QMAKE_LFLAGS   += $$QMAKE_LFLAGS_OPENMP

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}
