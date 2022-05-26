QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    src/elemental_profile.cpp \
    src/elemental_profile_set.cpp

HEADERS += \
    include/elemental_profile.h \
    include/elemental_profile_set.h \
    mainwindow.h

INCLUDEPATH += include/

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    CMBSource_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
