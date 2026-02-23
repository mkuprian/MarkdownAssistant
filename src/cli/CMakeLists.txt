# =============================================================================
# cli - Command Line Tools
# =============================================================================

# =============================================================================
# mdcli - GapBuffer Demo
# =============================================================================
add_executable(mdcli)

# Add sources using modern CMake target_sources
target_sources(mdcli
    PRIVATE
        main.cpp
)

# Specify C++ standard using target_compile_features
target_compile_features(mdcli
    PRIVATE
        cxx_std_17
)

# Link to the gapbuffer library
target_link_libraries(mdcli
    PRIVATE
        mdeditor::gapbuffer
)

# Set compiler warnings
target_compile_options(mdcli
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Set working directory for running the demo
set_target_properties(mdcli PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# =============================================================================
# mdpreview - Markdown Preview Generator
# =============================================================================
add_executable(mdpreview)

target_sources(mdpreview
    PRIVATE
        preview.cpp
)

target_compile_features(mdpreview
    PRIVATE
        cxx_std_17
)

# Link to gapbuffer and markdown libraries
target_link_libraries(mdpreview
    PRIVATE
        mdeditor::gapbuffer
        mdeditor::markdown
)

target_compile_options(mdpreview
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

set_target_properties(mdpreview PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
