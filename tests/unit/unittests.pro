# SedSAT3 – Sediment Source Assessment Tool, version 3
# U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.
#
# Unit tests (Qt Test) for the analysis code.
#
#   qmake tests/unit/unittests.pro && make && ./sedsat3-unittests
#
# The tests run without a display (they select Qt's offscreen platform).

QT += core gui widgets testlib
CONFIG += console testcase
CONFIG -= app_bundle

TARGET = sedsat3-unittests
TEMPLATE = app

DEFINES += QT_NO_DEBUG_OUTPUT

include(../../sedsat3_core.pri)

# The GA's bit-string class is tested whether or not the GA is built
!ga {
    INCLUDEPATH += $$SEDSAT3_ROOT/include/GA
    SOURCES += \
        $$SEDSAT3_ROOT/src/GA/Binary.cpp \
        $$SEDSAT3_ROOT/src/GA/GADistribution.cpp
}

# The element table and its "Include all elements" check box
SOURCES += \
    $$SEDSAT3_ROOT/elementstablemodel.cpp \
    $$SEDSAT3_ROOT/formelementinformation.cpp
HEADERS += \
    $$SEDSAT3_ROOT/elementstablemodel.h \
    $$SEDSAT3_ROOT/formelementinformation.h
FORMS += \
    $$SEDSAT3_ROOT/formelementinformation.ui

SOURCES += tst_sedsat3.cpp
