QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Utilities/Matrix.cpp \
    ../Utilities/Matrix_arma.cpp \
    ../Utilities/Matrix_arma_sp.cpp \
    ../Utilities/NormalDist.cpp \
    ../Utilities/QuickSort.cpp \
    ../Utilities/Utilities.cpp \
    ../Utilities/Vector.cpp \
    ../Utilities/Vector_arma.cpp \
    ../qcustomplot6/qcustomplot6/qcustomplot.cpp \
    customplotbar.cpp \
    generalplotter.cpp \
    indicatesheetsdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwindow.cpp \
    src/elemental_profile.cpp \
    src/elemental_profile_set.cpp \
    src/sourcesinkdata.cpp

HEADERS += \
    ../Utilities/BTC.h \
    ../Utilities/BTC.hpp \
    ../Utilities/BTCSet.h \
    ../Utilities/BTCSet.hpp \
    ../Utilities/Matrix.h \
    ../Utilities/Matrix_arma.h \
    ../Utilities/Matrix_arma_sp.h \
    ../Utilities/NormalDist.h \
    ../Utilities/QuickSort.h \
    ../Utilities/Utilities.h \
    ../Utilities/Vector.h \
    ../Utilities/Vector_arma.h \
    ../qcustomplot6/qcustomplot6/qcustomplot.h \
    customplotbar.h \
    generalplotter.h \
    include/elemental_profile.h \
    include/elemental_profile_set.h \
    include/sourcesinkdata.h \
    indicatesheetsdialog.h \
    mainwindow.h \
    plotwindow.h



INCLUDEPATH += include/
INCLUDEPATH += ../QXlsx/QXlsx/header/
INCLUDEPATH += ../qcustomplot6/qcustomplot6
INCLUDEPATH += ../Utilities/


LIBS += /home/arash/Projects/QXlsx/build-QXlsx-Desktop_Qt_5_15_2_GCC_64bit-Release/libQXlsx.a
LIBS += -L"/usr/local/lib/ -lsuperlu.so"

FORMS += \
    indicatesheetsdialog.ui \
    mainwindow.ui \
    plotwindow.ui

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
