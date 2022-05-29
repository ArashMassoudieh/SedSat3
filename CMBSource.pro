QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../qcustomplot6/qcustomplot6/qcustomplot.cpp \
    indicatesheetsdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwindow.cpp \
    src/elemental_profile.cpp \
    src/elemental_profile_set.cpp \
    src/sourcesinkdata.cpp

HEADERS += \
    ../qcustomplot6/qcustomplot6/qcustomplot.h \
    include/elemental_profile.h \
    include/elemental_profile_set.h \
    include/sourcesinkdata.h \
    indicatesheetsdialog.h \
    mainwindow.h \
    plotwindow.h

INCLUDEPATH += include/
INCLUDEPATH += ../QXlsx/QXlsx/header/
INCLUDEPATH += ../qcustomplot6/qcustomplot6

LIBS += /home/arash/Projects/QXlsx/build-QXlsx-Desktop_Qt_5_15_2_GCC_64bit-Release/libQXlsx.a

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
