QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport charts

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += _arma
DEFINES += Q_version
SOURCES += \
    ../Utilities/Distribution.cpp \
    ../Utilities/Matrix.cpp \
    ../Utilities/Matrix_arma.cpp \
    ../Utilities/Matrix_arma_sp.cpp \
    ../Utilities/QuickSort.cpp \
    ../Utilities/Utilities.cpp \
    ../Utilities/Vector.cpp \
    ../Utilities/Vector_arma.cpp \
    ../qcustomplot6/qcustomplot.cpp \
    ProgressWindow.cpp \
    contribution.cpp \
    customplotbar.cpp \
    elementstablemodel.cpp \
    elementtabledelegate.cpp \
    filebrowserpushbuttom.cpp \
    formelementinformation.cpp \
    generalplotter.cpp \
    genericform.cpp \
    indicatesheetsdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwindow.cpp \
    results.cpp \
    resultswindow.cpp \
    src/GA/Binary.cpp \
    src/GA/GADistribution.cpp \
    src/GA/Individual.cpp \
    src/concentrationset.cpp \
    src/conductor.cpp \
    src/distribution.cpp \
    src/elemental_profile.cpp \
    src/elemental_profile_set.cpp \
    src/interface.cpp \
    src/observation.cpp \
    src/parameter.cpp \
    src/sourcesinkdata.cpp

HEADERS += \
    ../Utilities/BTC.h \
    ../Utilities/BTC.hpp \
    ../Utilities/BTCSet.h \
    ../Utilities/BTCSet.hpp \
    ../Utilities/Distribution.h \
    ../Utilities/Matrix.h \
    ../Utilities/Matrix_arma.h \
    ../Utilities/Matrix_arma_sp.h \
    ../Utilities/QuickSort.h \
    ../Utilities/Utilities.h \
    ../Utilities/Vector.h \
    ../Utilities/Vector_arma.h \
    ../qcustomplot6/qcustomplot.h \
    ProgressWindow.h \
    contribution.h \
    customplotbar.h \
    elementstablemodel.h \
    elementtabledelegate.h \
    filebrowserpushbuttom.h \
    formelementinformation.h \
    generalplotter.h \
    genericform.h \
    include/GA/Binary.h \
    include/GA/GA.h \
    include/GA/GA.hpp \
    include/GA/GADistribution.h \
    include/GA/Individual.h \
    include/concentrationset.h \
    include/conductor.h \
    include/elemental_profile.h \
    include/elemental_profile_set.h \
    include/interface.h \
    include/observation.h \
    include/parameter.h \
    include/sourcesinkdata.h \
    indicatesheetsdialog.h \
    mainwindow.h \
    plotwindow.h \
    results.h \
    resultswindow.h



INCLUDEPATH += include/
INCLUDEPATH += ../QXlsx/QXlsx/header/
INCLUDEPATH += ../qcustomplot6/
INCLUDEPATH += ../Utilities/
INCLUDEPATH += /usr/include/
INCLUDEPATH += /usr/include/x86_64-linux-gnu/
INCLUDEPATH += include/GA/
INCLUDEPATH += include/MCMC/

LIBS += /home/arash/Projects/QXlsx/libQXlsx.a
LIBS += -L"/usr/local/lib/ -lsuperlu.so"

FORMS += \
    ProgressWindow.ui \
    formelementinformation.ui \
    genericform.ui \
    indicatesheetsdialog.ui \
    mainwindow.ui \
    plotwindow.ui \
    resultswindow.ui

TRANSLATIONS += \
    CMBSource_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


CONFIG(debug, debug|release) {
    message(Building in debug mode)
    #QMAKE_CXXFLAGS+= -fopenmp
    #QMAKE_LFLAGS +=  -fopenmp
    ! macx: LIBS += -lgomp -lpthread -lgsl -larmadillo
    macx: LIBS += -lpthread
    DEFINES += NO_OPENMP DEBUG

} else {
    message(Building in release mode)
    !macx:QMAKE_CXXFLAGS += -fopenmp
    !macx:QMAKE_LFLAGS +=  -fopenmp
    # QMAKE_CFLAGS+=-pg
    # QMAKE_CXXFLAGS+=-pg
    # QMAKE_LFLAGS+=-pg
    DEFINES += ARMA_USE_OPENMP
    macx: DEFINES += NO_OPENMP
    ! macx: LIBS += -lgomp -lpthread -lgsl -larmadillo
    macx: LIBS += -lpthread
}

win32 {

    LAPACK_INCLUDE = $$PWD/include
    #64 bits build
    contains(QMAKE_TARGET.arch, x86_64) {
        #debug
        CONFIG(debug, debug|release) {
            LAPACK_LIB_DIR = $$PWD/libs/lapack-blas_lib_win64/debug
            LIBS +=  -L$${LAPACK_LIB_DIR} -llapack_win64_MTd \
                    -lblas_win64_MTd
        }
        #release
        CONFIG(release, debug|release) {
            LAPACK_LIB_DIR = $$PWD/libs/lapack-blas_lib_win64/release
            LIBS +=  -L$${LAPACK_LIB_DIR} -llapack_win64_MT \
                    -lblas_win64_MT
        }
    }

    INCLUDEPATH += $${LAPACK_INCLUDE}

    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS

}

linux {
    #sudo apt-get install libblas-dev liblapack-dev
     DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS
     LIBS += -larmadillo -llapack -lblas
}

macx {
    #sudo apt-get install libblas-dev liblapack-dev

     DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS
     LIBS += -lblas -llapack
     LIBS += -L$$PWD/../Armadillo/ -larmadillo.11.2.3
     INCLUDEPATH += $$PWD/../Armadillo/include/
     DEPENDPATH += $$PWD/../Armadillo
     INCLUDEPATH += /opt/homebrew/Cellar/gsl/2.7.1/include/
}

