# =============================================================================
# mdapp_stub - Console application stub
# =============================================================================

# Define the executable target
add_executable(mdapp_stub)

# Add sources using modern CMake target_sources
target_sources(mdapp_stub
    PRIVATE
        main.cpp
)

# Specify C++ standard using target_compile_features
target_compile_features(mdapp_stub
    PRIVATE
        cxx_std_17
)

# Link to the core library using modern CMake target_link_libraries
target_link_libraries(mdapp_stub
    PRIVATE
        mdeditor::mdcore_stub
)

# Set compiler warnings
target_compile_options(mdapp_stub
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
