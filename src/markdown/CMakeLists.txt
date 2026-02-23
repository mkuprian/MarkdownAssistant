# =============================================================================
# markdown - Pluggable Markdown Parser Library
# =============================================================================
#
# This library provides markdown-to-HTML rendering with a pluggable interface.
# By default, it uses FallbackRenderer which handles common markdown elements.
# When MD_USE_CMARK=ON is specified, it uses the cmark library for full
# CommonMark compliance.
#
# Usage:
#   target_link_libraries(your_target PRIVATE mdeditor::markdown)
#
# =============================================================================

# Define the static library target
add_library(markdown STATIC)

# Core sources (always included)
set(MARKDOWN_SOURCES
    fallback_renderer.cpp
    parser_factory.cpp
)

set(MARKDOWN_HEADERS
    IMarkdownParser.h
    fallback_renderer.h
    html_utils.h
)

# Add cmark adapter if MD_USE_CMARK is enabled
if(MD_USE_CMARK)
    list(APPEND MARKDOWN_SOURCES cmark_adapter.cpp)
    list(APPEND MARKDOWN_HEADERS cmark_adapter.h)
endif()

# Add sources
target_sources(markdown
    PRIVATE
        ${MARKDOWN_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            ${MARKDOWN_HEADERS}
)

# Specify C++ standard
target_compile_features(markdown
    PUBLIC
        cxx_std_17
)

# Include directories
target_include_directories(markdown
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Compiler warnings
target_compile_options(markdown
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# =============================================================================
# cmark integration (optional)
# =============================================================================
if(MD_USE_CMARK)
    include(FetchContent)
    
    # Fetch cmark from GitHub
    FetchContent_Declare(
        cmark
        GIT_REPOSITORY https://github.com/commonmark/cmark.git
        GIT_TAG        0.31.1
    )
    
    # Configure cmark build options
    set(CMARK_TESTS OFF CACHE BOOL "" FORCE)
    set(CMARK_SHARED OFF CACHE BOOL "" FORCE)
    set(CMARK_STATIC ON CACHE BOOL "" FORCE)
    set(CMARK_LIB_FUZZER OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(cmark)
    
    # Link to cmark static library
    target_link_libraries(markdown
        PRIVATE
            cmark_static
    )
    
    # Add include directory for cmark headers
    target_include_directories(markdown
        PRIVATE
            ${cmark_BINARY_DIR}/src  # For cmark_export.h
            ${cmark_SOURCE_DIR}/src
    )
    
    # Define MD_USE_CMARK for the preprocessor
    target_compile_definitions(markdown
        PUBLIC
            MD_USE_CMARK
    )
    
    message(STATUS "markdown: Using cmark for CommonMark compliance")
else()
    message(STATUS "markdown: Using FallbackRenderer (set -DMD_USE_CMARK=ON for full CommonMark)")
endif()

# Add alias for consistent usage
add_library(mdeditor::markdown ALIAS markdown)
