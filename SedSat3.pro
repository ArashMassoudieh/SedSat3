QT += core gui widgets printsupport charts

CONFIG += c++17
DEFINES += _arma SUPPORT_USE_QJSON GSL Q_GUI_SUPPORT

greaterThan(QT_MAJOR_VERSION, 5): DEFINES += Qt6

QT_VERSION = $$[QT_VERSION]
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore/$${QT_VERSION}/QtCore/private
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtGui/$${QT_VERSION}/QtGui/private


SOURCES += \
    Utilities/Distribution.cpp \
    Utilities/Matrix.cpp \
    Utilities/Matrix_arma.cpp \
    Utilities/Matrix_arma_sp.cpp \
    Utilities/QuickSort.cpp \
    Utilities/Utilities.cpp \
    Utilities/Vector.cpp \
    Utilities/Vector_arma.cpp \
    thirdparty/qcustomplot6/qcustomplot.cpp \
    FilePushButton.cpp \
    ProgressWindow.cpp \
    aboutdialog.cpp \
    chart.cpp \
    chartview.cpp \
    cmbmatrix.cpp \
    cmbtimeseries.cpp \
    cmbtimeseriesset.cpp \
    cmbvector.cpp \
    cmbvectorset.cpp \
    cmbvectorsetset.cpp \
    contribution.cpp \
    customplotbar.cpp \
    dialogchooseexcelsheets.cpp \
    elementstablemodel.cpp \
    elementtabledelegate.cpp \
    filebrowserpushbuttom.cpp \
    formelementinformation.cpp \
    generalchart.cpp \
    generalplotter.cpp \
    genericform.cpp \
    indicatesheetsdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    multiplelinearregression.cpp \
    multiplelinearregressionset.cpp \
    omsizecorrectiontablemodel.cpp \
    plotwindow.cpp \
    range.cpp \
    rangeset.cpp \
    resultitem.cpp \
    results.cpp \
    resultsetitem.cpp \
    resultswindow.cpp \
    resulttableviewer.cpp \
    selectsampledelegate.cpp \
    selectsamples.cpp \
    selectsampletablemodel.cpp \
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
    testmcmc.cpp \
    toolboxitem.cpp

HEADERS += \
    Utilities/TimeSeries.h \
    Utilities/TimeSeries.hpp \
    Utilities/TimeSeriesSet.h \
    Utilities/TimeSeriesSet.hpp \
    Utilities/Distribution.h \
    Utilities/Matrix.h \
    Utilities/Matrix_arma.h \
    Utilities/Matrix_arma_sp.h \
    Utilities/QuickSort.h \
    Utilities/Utilities.h \
    Utilities/Vector.h \
    Utilities/Vector_arma.h \
    thirdparty/qcustomplot6/qcustomplot.h \
    FilePushButton.h \
    ProgressWindow.h \
    aboutdialog.h \
    chart.h \
    chartview.h \
    cmbmatrix.h \
    cmbtimeseries.h \
    cmbtimeseriesset.h \
    cmbvector.h \
    cmbvectorset.h \
    cmbvectorsetset.h \
    contribution.h \
    customplotbar.h \
    dialogchooseexcelsheets.h \
    elementstablemodel.h \
    elementtabledelegate.h \
    filebrowserpushbuttom.h \
    formelementinformation.h \
    generalchart.h \
    generalplotter.h \
    genericform.h \
    include/GA/Binary.h \
    include/GA/GA.h \
    include/GA/GA.hpp \
    include/GA/GADistribution.h \
    include/GA/Individual.h \
    include/MCMC/MCMC.h \
    include/MCMC/MCMC.hpp \
    include/cmbdistribution.h \
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
    multiplelinearregression.h \
    multiplelinearregressionset.h \
    omsizecorrectiontablemodel.h \
    plotwindow.h \
    range.h \
    rangeset.h \
    resultitem.h \
    results.h \
    resultsetitem.h \
    resultswindow.h \
    resulttableviewer.h \
    selectsampledelegate.h \
    selectsamples.h \
    selectsampletablemodel.h \
    testmcmc.h \
    toolboxitem.h



INCLUDEPATH += \
    include \
    include/GA \
    include/MCMC \
    Utilities \
    thirdparty/qcustomplot6 \
    thirdparty/QXlsx/QXlsx/header

# QXlsx (prebuilt static library)
INCLUDEPATH += thirdparty/QXlsx/QXlsx/header
LIBS += $$PWD/thirdparty/QXlsx/libQXlsx.a


# Libraries
unix:!macx {
    LIBS += -larmadillo -llapack -lblas -lgsl -lpthread
}
macx {
    LIBS += -larmadillo -llapack -lblas -lgsl -lpthread
}

# OpenMP
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_OPENMP
LIBS += $$QMAKE_LIBS_OPENMP

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

# OpenMP
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_OPENMP
QMAKE_LFLAGS   += $$QMAKE_LFLAGS_OPENMP

unix:!macx {
    LIBS += -lgomp -lpthread
}
macx {
    LIBS += -lomp -lpthread
}

