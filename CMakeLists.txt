# =============================================================================
# mdeditor - Qt-based Markdown Editor
# Root CMakeLists.txt
# =============================================================================
cmake_minimum_required(VERSION 3.21)

project(mdeditor
    VERSION 0.1.0
    DESCRIPTION "Qt-based Markdown Editor"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# C++ Standard Configuration
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE/tooling support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Project Options
# -----------------------------------------------------------------------------
option(BUILD_TESTING "Build unit tests" ON)
option(MD_USE_WEBENGINE "Use Qt WebEngine for preview rendering" OFF)
option(MD_USE_CMARK "Use cmark library for full CommonMark compliance" OFF)

# -----------------------------------------------------------------------------
# CMake Module Path
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# -----------------------------------------------------------------------------
# Qt6 Configuration (optional, for mdapp)
# -----------------------------------------------------------------------------
# Set Qt6 prefix path if not already set
# Users should set CMAKE_PREFIX_PATH to their Qt installation, e.g.:
#   cmake -D CMAKE_PREFIX_PATH=C:/Qt/6.10.1/msvc2022_arm64 ...
if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.10.1/msvc2022_arm64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_arm64")
endif()

# Try to find Qt6 (optional - mdapp will be skipped if not found)
find_package(Qt6 QUIET COMPONENTS Core Gui Qml Quick)
if(Qt6_FOUND)
    set(MDEDITOR_HAS_QT6 TRUE)
    message(STATUS "Qt6 found at: ${Qt6_DIR}")
else()
    set(MDEDITOR_HAS_QT6 FALSE)
    message(STATUS "Qt6 not found - mdapp will not be built")
    message(STATUS "  To enable, set CMAKE_PREFIX_PATH to your Qt6 installation")
endif()

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(tools)

# Conditionally add tests
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== mdeditor Configuration ===")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  BUILD_TESTING:  ${BUILD_TESTING}")
message(STATUS "  MD_USE_CMARK:   ${MD_USE_CMARK}")
message(STATUS "  MD_USE_WEBENGINE: ${MD_USE_WEBENGINE}")
message(STATUS "  Qt6 Available:    ${MDEDITOR_HAS_QT6}")
message(STATUS "==============================")
message(STATUS "")
