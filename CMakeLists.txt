#Update 9/22/2025

cmake_minimum_required(VERSION 3.16)
project(SedSat3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# Qt6 setup
# ----------------------------
find_package(Qt6 6.2 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    PrintSupport
    Charts
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ----------------------------
# Third-party: QXlsx
# ----------------------------
add_subdirectory(thirdparty/QXlsx/QXlsx)

# ----------------------------
# Sources, headers, forms
# ----------------------------
set(PROJECT_SOURCES
    Utilities/Distribution.cpp
    Utilities/Matrix.cpp
    Utilities/Matrix_arma.cpp
    Utilities/Matrix_arma_sp.cpp
    Utilities/QuickSort.cpp
    Utilities/Utilities.cpp
    Utilities/Vector.cpp
    Utilities/Vector_arma.cpp
    thirdparty/qcustomplot6/qcustomplot.cpp
    FilePushButton.cpp
    ProgressWindow.cpp
    aboutdialog.cpp
    chart.cpp
    chartview.cpp
    cmbmatrix.cpp
    cmbtimeseries.cpp
    cmbtimeseriesset.cpp
    cmbvector.cpp
    cmbvectorset.cpp
    cmbvectorsetset.cpp
    contribution.cpp
    customplotbar.cpp
    dialogchooseexcelsheets.cpp
    elementstablemodel.cpp
    elementtabledelegate.cpp
    filebrowserpushbuttom.cpp
    formelementinformation.cpp
    generalchart.cpp
    generalplotter.cpp
    genericform.cpp
    indicatesheetsdialog.cpp
    main.cpp
    mainwindow.cpp
    multiplelinearregression.cpp
    multiplelinearregressionset.cpp
    omsizecorrectiontablemodel.cpp
    plotwindow.cpp
    range.cpp
    rangeset.cpp
    resultitem.cpp
    results.cpp
    resultsetitem.cpp
    resultswindow.cpp
    resulttableviewer.cpp
    selectsampledelegate.cpp
    selectsamples.cpp
    selectsampletablemodel.cpp
    src/GA/Binary.cpp
    src/GA/GADistribution.cpp
    src/GA/Individual.cpp
    src/cmbdistribution.cpp
    src/concentrationset.cpp
    src/conductor.cpp
    src/elemental_profile.cpp
    src/elemental_profile_set.cpp
    src/interface.cpp
    src/observation.cpp
    src/parameter.cpp
    src/sourcesinkdata.cpp
    testmcmc.cpp
    toolboxitem.cpp
)

set(PROJECT_HEADERS
    Utilities/TimeSeries.h
    Utilities/TimeSeries.hpp
    Utilities/TimeSeriesSet.h
    Utilities/TimeSeriesSet.hpp
    Utilities/Distribution.h
    Utilities/Matrix.h
    Utilities/Matrix_arma.h
    Utilities/Matrix_arma_sp.h
    Utilities/QuickSort.h
    Utilities/Utilities.h
    Utilities/Vector.h
    Utilities/Vector_arma.h
    thirdparty/qcustomplot6/qcustomplot.h
    FilePushButton.h
    ProgressWindow.h
    aboutdialog.h
    chart.h
    chartview.h
    cmbmatrix.h
    cmbtimeseries.h
    cmbtimeseriesset.h
    cmbvector.h
    cmbvectorset.h
    cmbvectorsetset.h
    contribution.h
    customplotbar.h
    dialogchooseexcelsheets.h
    elementstablemodel.h
    elementtabledelegate.h
    filebrowserpushbuttom.h
    formelementinformation.h
    generalchart.h
    generalplotter.h
    genericform.h
    include/GA/Binary.h
    include/GA/GA.h
    include/GA/GA.hpp
    include/GA/GADistribution.h
    include/GA/Individual.h
    include/MCMC/MCMC.h
    include/MCMC/MCMC.hpp
    include/cmbdistribution.h
    include/concentrationset.h
    include/conductor.h
    include/elemental_profile.h
    include/elemental_profile_set.h
    include/interface.h
    include/observation.h
    include/parameter.h
    include/sourcesinkdata.h
    indicatesheetsdialog.h
    mainwindow.h
    multiplelinearregression.h
    multiplelinearregressionset.h
    omsizecorrectiontablemodel.h
    plotwindow.h
    range.h
    rangeset.h
    resultitem.h
    results.h
    resultsetitem.h
    resultswindow.h
    resulttableviewer.h
    selectsampledelegate.h
    selectsamples.h
    selectsampletablemodel.h
    testmcmc.h
    toolboxitem.h
)

set(UI_FILES
    ProgressWindow.ui
    aboutdialog.ui
    dialogchooseexcelsheets.ui
    formelementinformation.ui
    generalchart.ui
    genericform.ui
    indicatesheetsdialog.ui
    mainwindow.ui
    plotwindow.ui
    resultswindow.ui
    resulttableviewer.ui
    selectsamples.ui
)

# ----------------------------
# Executable target
# ----------------------------
qt_add_executable(SedSat3
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${UI_FILES}
)

# ----------------------------
# Include directories
# ----------------------------
target_include_directories(SedSat3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GA
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MCMC
    ${CMAKE_CURRENT_SOURCE_DIR}/Utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qcustomplot6
)

# ----------------------------
# Libraries
# ----------------------------
find_package(Armadillo REQUIRED)
find_package(GSL REQUIRED)
find_package(OpenMP REQUIRED)

target_link_libraries(SedSat3 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Charts
    QXlsx
    ${ARMADILLO_LIBRARIES}
    GSL::gsl GSL::gslcblas
    OpenMP::OpenMP_CXX
)

# ----------------------------
# Definitions
# ----------------------------
target_compile_definitions(SedSat3 PRIVATE
    _arma
    SUPPORT_USE_QJSON
    GSL
    Q_GUI_SUPPORT
    Qt6
)

# ----------------------------
# Build output (local builds only)
# ----------------------------
set_target_properties(SedSat3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

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

# Install rules

# Executable → /usr/local/sedsat3/bin/SedSat3
install(TARGETS SedSat3
    RUNTIME DESTINATION bin
)

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
set(CPACK_PACKAGE_VERSION "1.0.25")
set(CPACK_PACKAGE_CONTACT "Arash Massoudieh <massoudieh@cua.edu>")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Arash Massoudieh") # required
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

