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
message(STATUS "==============================")
message(STATUS "")
