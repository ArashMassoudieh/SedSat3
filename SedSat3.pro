# Graphical build of SedSAT3.
#
# The analysis code shared with the command line program is listed in
# sedsat3_core.pri; this file adds the interface, plotting and Excel import.
#
#   qmake SedSat3.pro && make             (CONFIG+=ga to include the GA)

QT += core gui widgets printsupport charts

DEFINES += USE_QCHARTS

include(sedsat3_core.pri)

# QXlsx is compiled with the program from the thirdparty/QXlsx submodule on
# every platform. (Linux used to link a prebuilt libQXlsx.a, which no longer
# matched the installed Qt and is not part of the repository.) It needs Qt's
# private headers, which the Qt online installer provides.
include(thirdparty/QXlsx/QXlsx/QXlsx.pri)
# QXlsx.pri adds the gui-private module, which on Linux also links
# libxkbcommon; QXlsx needs only the private headers (for QZipReader, whose
# symbols are in QtGui), so the module is dropped and the headers added
QT -= gui-private
QT_VERSION = $$[QT_VERSION]
INCLUDEPATH += \
    $$[QT_INSTALL_HEADERS]/QtCore/$${QT_VERSION} \
    $$[QT_INSTALL_HEADERS]/QtCore/$${QT_VERSION}/QtCore \
    $$[QT_INSTALL_HEADERS]/QtGui/$${QT_VERSION} \
    $$[QT_INSTALL_HEADERS]/QtGui/$${QT_VERSION}/QtGui

INCLUDEPATH += \
    thirdparty/qcustomplot6 \
    thirdparty/QXlsx/QXlsx/header

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    guihost.cpp \
    aboutdialog.cpp \
    chart.cpp \
    chartview.cpp \
    dialogchooseexcelsheets.cpp \
    elementstablemodel.cpp \
    elementtabledelegate.cpp \
    filebrowserpushbutton.cpp \
    formelementinformation.cpp \
    generalchart.cpp \
    GeneralChartPlotter.cpp \
    generalplotter.cpp \
    genericform.cpp \
    indicatesheetsdialog.cpp \
    omsizecorrectiontablemodel.cpp \
    optionsdialog.cpp \
    plotwindow.cpp \
    ProgressWindow.cpp \
    resultswindow.cpp \
    resulttableviewer.cpp \
    selectsampledelegate.cpp \
    selectsamples.cpp \
    selectsampletablemodel.cpp \
    toolboxitem.cpp \
    thirdparty/qcustomplot6/qcustomplot.cpp

HEADERS += \
    mainwindow.h \
    guihost.h \
    aboutdialog.h \
    chart.h \
    chartview.h \
    dialogchooseexcelsheets.h \
    elementstablemodel.h \
    elementtabledelegate.h \
    filebrowserpushbutton.h \
    formelementinformation.h \
    generalchart.h \
    GeneralChartPlotter.h \
    generalplotter.h \
    genericform.h \
    indicatesheetsdialog.h \
    omsizecorrectiontablemodel.h \
    optionsdialog.h \
    plotwindow.h \
    ProgressWindow.h \
    resultswindow.h \
    resulttableviewer.h \
    selectsampledelegate.h \
    selectsamples.h \
    selectsampletablemodel.h \
    toolboxitem.h \
    thirdparty/qcustomplot6/qcustomplot.h

FORMS += \
    ProgressWindow.ui \
    dialogchooseexcelsheets.ui \
    formelementinformation.ui \
    generalchart.ui \
    genericform.ui \
    indicatesheetsdialog.ui \
    mainwindow.ui \
    plotwindow.ui \
    resultswindow.ui \
    resulttableviewer.ui \
    selectsamples.ui

macx {
    # Copy resources folder into the .app bundle after each build
    # Lands at SedSat3.app/resources/ to match the existing applicationDirPath() + "/../../resources" code
    COPY_RESOURCES.commands = $(COPY_DIR) \"$$PWD/resources/\" \"$$OUT_PWD/$${TARGET}.app/Contents/Resources/\"
    COPY_RESOURCES.depends = FORCE
    QMAKE_EXTRA_TARGETS += COPY_RESOURCES
    POST_TARGETDEPS += COPY_RESOURCES
}
