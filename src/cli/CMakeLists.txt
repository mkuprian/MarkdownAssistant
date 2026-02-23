# =============================================================================
# cli - GapBuffer Command Line Demo
# =============================================================================

# Define the executable target
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
