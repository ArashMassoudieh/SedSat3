# SedSAT3 – Sediment Source Assessment Tool, version 3
# U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.
#
# Targets: SedSat3 (graphical program), sedsat3-cli (command line program),
# sedsat3-unittests (Qt Test, run with ctest).
#
# Options:
#   -DSEDSAT3_BUILD_GUI=OFF  build only the command line program and tests
#   -DENABLE_GA=ON           build the genetic-algorithm solver and its commands.
#                            The GA works but is left out by default:
#                            Levenberg-Marquardt solves the same problem more
#                            efficiently, and offering two deterministic methods
#                            would confuse users.
#   -DBUILD_TESTING=OFF      skip the unit tests

cmake_minimum_required(VERSION 3.16)
project(SedSat3 VERSION 1.1.8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clang-tidy

option(SEDSAT3_BUILD_GUI "Build the graphical program" ON)
option(ENABLE_GA "Build the genetic-algorithm solver" OFF)
include(CTest)

# ----------------------------
# Qt6
# ----------------------------
set(SEDSAT3_QT_COMPONENTS Core Gui Widgets)
if(SEDSAT3_BUILD_GUI)
    list(APPEND SEDSAT3_QT_COMPONENTS PrintSupport Charts)
endif()
if(BUILD_TESTING)
    list(APPEND SEDSAT3_QT_COMPONENTS Test)
endif()
find_package(Qt6 6.2 REQUIRED COMPONENTS ${SEDSAT3_QT_COMPONENTS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Armadillo REQUIRED)
find_package(GSL REQUIRED)
find_package(OpenMP REQUIRED)

# ----------------------------
# Analysis code shared by every target (the same list as sedsat3_core.pri)
# ----------------------------
set(SEDSAT3_CORE_SOURCES
    Utilities/Distribution.cpp
    Utilities/Matrix.cpp
    Utilities/Matrix_arma.cpp
    Utilities/Matrix_arma_sp.cpp
    Utilities/QuickSort.cpp
    Utilities/Utilities.cpp
    Utilities/Vector.cpp
    Utilities/Vector_arma.cpp
    src/cmbdistribution.cpp
    src/concentrationset.cpp
    src/conductor.cpp
    src/elemental_profile.cpp
    src/elemental_profile_set.cpp
    src/interface.cpp
    src/numericparsing.cpp
    src/observation.cpp
    src/parameter.cpp
    src/sourcesinkdata.cpp
    cmbmatrix.cpp
    cmbtimeseries.cpp
    cmbtimeseriesset.cpp
    cmbvector.cpp
    cmbvectorset.cpp
    cmbvectorsetset.cpp
    contribution.cpp
    multiplelinearregression.cpp
    multiplelinearregressionset.cpp
    range.cpp
    rangeset.cpp
    resultitem.cpp
    results.cpp
    resultsetitem.cpp
    testmcmc.cpp
)

set(SEDSAT3_CORE_HEADERS
    Utilities/Distribution.h
    Utilities/Matrix.h
    Utilities/Matrix_arma.h
    Utilities/Matrix_arma_sp.h
    Utilities/QuickSort.h
    Utilities/TimeSeries.h
    Utilities/TimeSeries.hpp
    Utilities/TimeSeriesSet.h
    Utilities/TimeSeriesSet.hpp
    Utilities/Utilities.h
    Utilities/Vector.h
    Utilities/Vector_arma.h
    include/analysishost.h
    include/cmbdistribution.h
    include/concentrationset.h
    include/conductor.h
    include/elemental_profile.h
    include/elemental_profile_set.h
    include/interface.h
    include/MCMC/MCMC.h
    include/MCMC/MCMC.hpp
    include/numericparsing.h
    include/observation.h
    include/parameter.h
    include/progressreporter.h
    include/randomstream.h
    include/mcmcdiagnostics.h
    include/sourcesinkdata.h
)

set(SEDSAT3_GA_SOURCES
    src/GA/Binary.cpp
    src/GA/GADistribution.cpp
    src/GA/Individual.cpp
)
set(SEDSAT3_GA_HEADERS
    include/GA/Binary.h
    include/GA/GA.h
    include/GA/GA.hpp
    include/GA/GADistribution.h
    include/GA/Individual.h
)
if(ENABLE_GA)
    list(APPEND SEDSAT3_CORE_SOURCES ${SEDSAT3_GA_SOURCES})
    list(APPEND SEDSAT3_CORE_HEADERS ${SEDSAT3_GA_HEADERS})
endif()

# Settings every target shares
function(sedsat3_configure target)
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MCMC
        ${CMAKE_CURRENT_SOURCE_DIR}/Utilities
        ${ARMADILLO_INCLUDE_DIRS}
    )
    target_compile_definitions(${target} PRIVATE _arma SUPPORT_USE_QJSON GSL Q_GUI_SUPPORT Qt6)
    if(ENABLE_GA)
        target_compile_definitions(${target} PRIVATE SEDSAT3_ENABLE_GA)
        target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/GA)
    endif()
    # A non-void function that falls off its end is undefined behaviour; make
    # it a build error rather than a warning (USGS code review, Oct 2026)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=return-type>
        $<$<CXX_COMPILER_ID:MSVC>:/we4715 /Zc:__cplusplus /utf-8>
    )
    if(MSVC)
        target_compile_definitions(${target} PRIVATE NOMINMAX)
    endif()
    target_link_libraries(${target} PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        ${ARMADILLO_LIBRARIES}
        GSL::gsl GSL::gslcblas
        OpenMP::OpenMP_CXX
    )
endfunction()

# ----------------------------
# Graphical program
# ----------------------------
if(SEDSAT3_BUILD_GUI)
    add_subdirectory(thirdparty/QXlsx/QXlsx)

    qt_add_executable(SedSat3
        ${SEDSAT3_CORE_SOURCES} ${SEDSAT3_CORE_HEADERS}
        main.cpp mainwindow.cpp mainwindow.h
        guihost.cpp guihost.h
        aboutdialog.cpp aboutdialog.h
        chart.cpp chart.h
        chartview.cpp chartview.h
        dialogchooseexcelsheets.cpp dialogchooseexcelsheets.h
        elementstablemodel.cpp elementstablemodel.h
        elementtabledelegate.cpp elementtabledelegate.h
        excelimport.cpp excelimport.h
        filebrowserpushbutton.cpp filebrowserpushbutton.h
        formelementinformation.cpp formelementinformation.h
        generalchart.cpp generalchart.h
        GeneralChartPlotter.cpp GeneralChartPlotter.h
        generalplotter.cpp generalplotter.h
        genericform.cpp genericform.h
        indicatesheetsdialog.cpp indicatesheetsdialog.h
        omsizecorrectiontablemodel.cpp omsizecorrectiontablemodel.h
        optionsdialog.cpp optionsdialog.h
        plotwindow.cpp plotwindow.h
        ProgressWindow.cpp ProgressWindow.h
        resultswindow.cpp resultswindow.h
        resulttableviewer.cpp resulttableviewer.h
        selectsampledelegate.cpp selectsampledelegate.h
        selectsamples.cpp selectsamples.h
        selectsampletablemodel.cpp selectsampletablemodel.h
        toolboxitem.cpp toolboxitem.h
        thirdparty/qcustomplot6/qcustomplot.cpp thirdparty/qcustomplot6/qcustomplot.h
        ProgressWindow.ui dialogchooseexcelsheets.ui formelementinformation.ui
        generalchart.ui genericform.ui indicatesheetsdialog.ui mainwindow.ui
        plotwindow.ui resultswindow.ui resulttableviewer.ui selectsamples.ui
    )
    sedsat3_configure(SedSat3)
    target_include_directories(SedSat3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qcustomplot6)
    target_compile_definitions(SedSat3 PRIVATE USE_QCHARTS)
    target_link_libraries(SedSat3 PRIVATE Qt6::PrintSupport Qt6::Charts QXlsx)
    set_target_properties(SedSat3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# ----------------------------
# Command line program
# ----------------------------
add_executable(sedsat3-cli
    ${SEDSAT3_CORE_SOURCES} ${SEDSAT3_CORE_HEADERS}
    cli/main_cli.cpp cli/consolehost.cpp cli/consolehost.h
    cli/commandcatalog.cpp cli/commandcatalog.h
    cli/setupcommands.cpp cli/setupcommands.h
    cli/scriptrunner.cpp cli/scriptrunner.h
)
sedsat3_configure(sedsat3-cli)
target_include_directories(sedsat3-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cli)
target_compile_definitions(sedsat3-cli PRIVATE QT_NO_DEBUG_OUTPUT)
set_target_properties(sedsat3-cli PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ----------------------------
# Unit tests
# ----------------------------
if(BUILD_TESTING)
    set(SEDSAT3_TEST_SOURCES
        tests/unit/tst_sedsat3.cpp
        elementstablemodel.cpp elementstablemodel.h
        elementtabledelegate.cpp elementtabledelegate.h
        formelementinformation.cpp formelementinformation.h formelementinformation.ui
    )
    if(NOT ENABLE_GA)
        # The GA's bit-string class is tested whether or not the GA is built
        list(APPEND SEDSAT3_TEST_SOURCES src/GA/Binary.cpp src/GA/GADistribution.cpp)
    endif()
    add_executable(sedsat3-unittests ${SEDSAT3_CORE_SOURCES} ${SEDSAT3_CORE_HEADERS} ${SEDSAT3_TEST_SOURCES})
    sedsat3_configure(sedsat3-unittests)
    target_include_directories(sedsat3-unittests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/GA)
    target_compile_definitions(sedsat3-unittests PRIVATE QT_NO_DEBUG_OUTPUT)
    target_link_libraries(sedsat3-unittests PRIVATE Qt6::Test)
    if(SEDSAT3_BUILD_GUI)
        # The Excel import needs QXlsx, which is built with the GUI
        target_sources(sedsat3-unittests PRIVATE excelimport.cpp excelimport.h)
        target_link_libraries(sedsat3-unittests PRIVATE QXlsx)
        target_compile_definitions(sedsat3-unittests PRIVATE SEDSAT3_TEST_EXCEL)
    endif()
    add_test(NAME unit COMMAND sedsat3-unittests)
    set_tests_properties(unit PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # Regression: the deterministic analyses against tests/baseline/
    find_program(BASH_EXECUTABLE bash)
    find_package(Python3 COMPONENTS Interpreter)
    if(BASH_EXECUTABLE AND Python3_FOUND)
        add_test(NAME regression
                 COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/run.sh
                         $<TARGET_FILE:sedsat3-cli> ${CMAKE_BINARY_DIR}/regression)
        set_tests_properties(regression PROPERTIES ENVIRONMENT "PYTHON=${Python3_EXECUTABLE}")
    endif()
endif()

# Install and packaging rules use Linux paths
if(UNIX AND NOT APPLE)

# ----------------------------
# Install + packaging setup
# ----------------------------

# Default install location → /usr/local/sedsat3
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local/sedsat3" CACHE PATH "install path" FORCE)
endif()

# Tell CPack to honor the prefix
set(CPACK_SET_DESTDIR ON)
set(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

if(SEDSAT3_BUILD_GUI)
    # Executable → /usr/local/sedsat3/bin/SedSat3
    install(TARGETS SedSat3 RUNTIME DESTINATION bin)
endif()
install(TARGETS sedsat3-cli RUNTIME DESTINATION bin)

# Resources → /usr/local/sedsat3/resources
# Place resources two levels above the binary (/usr/local/resources)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
    DESTINATION ../resources
)

# Desktop entry → /usr/share/applications
install(FILES ${CMAKE_SOURCE_DIR}/resources/sedsat3.desktop
    DESTINATION /usr/share/applications
)

# App icon → /usr/share/icons
install(FILES ${CMAKE_SOURCE_DIR}/resources/icons/sedsat3.png
    DESTINATION /usr/share/icons/hicolor/256x256/apps
)

# ----------------------------
# Packaging metadata
# ----------------------------
set(CPACK_PACKAGE_NAME "sedsat3")
set(CPACK_PACKAGE_VERSION "1.1.8")
set(CPACK_PACKAGE_CONTACT "Arash Massoudieh <massoudieh@cua.edu>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Arash Massoudieh") # required
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

endif()
