# =============================================================================
# src/mdapp/CMakeLists.txt - Qt6 QML Markdown Editor
# =============================================================================
#
# This CMakeLists.txt builds the main mdapp executable with Qt6 integration.
#
# CONFIGURATION OPTIONS (from parent CMake):
# ------------------------------------------
#   MD_USE_WEBENGINE=ON   - Enable Qt WebEngine for HTML preview (requires x64)
#                           Default: OFF (uses Text with rich text instead)
#
# DEPENDENCIES:
# -------------
#   Qt6::Core, Qt6::Gui, Qt6::Qml, Qt6::Quick, Qt6::QuickControls2
#   Optional: Qt6::WebEngineQuick (when MD_USE_WEBENGINE=ON)
#
# =============================================================================

# -----------------------------------------------------------------------------
# Find Qt6 Packages
# -----------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

if(MD_USE_WEBENGINE)
    find_package(Qt6 COMPONENTS WebEngineQuick QUIET)
    if(Qt6WebEngineQuick_FOUND)
        message(STATUS "mdapp: Qt6 WebEngine found - enabling HTML preview with WebEngineView")
    else()
        message(WARNING "mdapp: Qt6 WebEngine not found (requires x64), falling back to Text preview")
        set(MD_USE_WEBENGINE OFF CACHE BOOL "Use Qt WebEngine for preview" FORCE)
    endif()
endif()

# -----------------------------------------------------------------------------
# Qt6 Project Setup
# -----------------------------------------------------------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -----------------------------------------------------------------------------
# Source Files
# -----------------------------------------------------------------------------
set(MDAPP_SOURCES
    main.cpp
    DocumentController.cpp
    DocumentController.h
)

set(MDAPP_RESOURCES
    qml.qrc
)

# -----------------------------------------------------------------------------
# Executable Target
# -----------------------------------------------------------------------------
qt_add_executable(mdapp
    ${MDAPP_SOURCES}
    ${MDAPP_RESOURCES}
)

# Set output name
set_target_properties(mdapp PROPERTIES
    OUTPUT_NAME "mdapp"
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# -----------------------------------------------------------------------------
# Include Directories
# -----------------------------------------------------------------------------
target_include_directories(mdapp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# -----------------------------------------------------------------------------
# Link Libraries
# -----------------------------------------------------------------------------
target_link_libraries(mdapp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    mdeditor::gapbuffer
    mdeditor::markdown
)

# Add WebEngine if available
if(MD_USE_WEBENGINE AND Qt6WebEngineQuick_FOUND)
    target_link_libraries(mdapp PRIVATE Qt6::WebEngineQuick)
    target_compile_definitions(mdapp PRIVATE MD_USE_WEBENGINE)
endif()

# -----------------------------------------------------------------------------
# Compile Features
# -----------------------------------------------------------------------------
target_compile_features(mdapp PRIVATE cxx_std_17)

# -----------------------------------------------------------------------------
# Qt Deployment (for packaging)
# -----------------------------------------------------------------------------
# On Windows, copy Qt DLLs to output directory for development
if(WIN32)
    # Use windeployqt in post-build for easier development
    # This can be enabled for packaging:
    # add_custom_command(TARGET mdapp POST_BUILD
    #     COMMAND ${Qt6_DIR}/../../../bin/windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR}/qml $<TARGET_FILE:mdapp>
    #     COMMENT "Running windeployqt..."
    # )
endif()

# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
message(STATUS "mdapp: Qt6 QML markdown editor configured")
message(STATUS "  - WebEngine: ${MD_USE_WEBENGINE}")
