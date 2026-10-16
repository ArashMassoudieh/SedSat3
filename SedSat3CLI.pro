# Command line build of SedSAT3.
#
# Runs the same analyses as the graphical application by driving the same
# Conductor, but reports through a console host instead of windows. The
# analysis code it shares with the graphical program is listed in
# sedsat3_core.pri; this file adds only the console front end.
#
#   qmake SedSat3CLI.pro && make          (CONFIG+=ga to include the GA)
#   ./sedsat3-cli script.json
#
# QtWidgets is still linked, because Interface::ToTable() returns a
# QTableWidget and Interface is a base of every result type. No widget is ever
# created, so the program runs with QCoreApplication and needs no display.

QT += core gui widgets
CONFIG += console
CONFIG -= app_bundle

TARGET = sedsat3-cli
TEMPLATE = app

# qDebug() tracing was written for the GUI, where it goes nowhere. On a console
# it ran to tens of megabytes per MCMC run. The project's own calls are now
# commented out; this also silences any that remain in Utilities.
# qWarning() and above are unaffected.
DEFINES += QT_NO_DEBUG_OUTPUT

include(sedsat3_core.pri)

INCLUDEPATH += cli

SOURCES += \
    cli/main_cli.cpp \
    cli/consolehost.cpp \
    cli/commandcatalog.cpp \
    cli/setupcommands.cpp \
    cli/scriptrunner.cpp

HEADERS += \
    cli/consolehost.h \
    cli/commandcatalog.h \
    cli/setupcommands.h \
    cli/scriptrunner.h
