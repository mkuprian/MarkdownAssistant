# =============================================================================
# mdcore_stub - Core Markdown library (stub implementation)
# =============================================================================

# Define the static library target
add_library(mdcore_stub STATIC)

# Add sources using modern CMake target_sources
target_sources(mdcore_stub
    PRIVATE
        mdcore_stub.cpp
)

# Specify C++ standard using target_compile_features (modern CMake idiom)
target_compile_features(mdcore_stub
    PUBLIC
        cxx_std_17
)

# Set include directories for consumers
# PUBLIC: available to both this target and consumers
target_include_directories(mdcore_stub
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Set compiler warnings (optional but recommended)
target_compile_options(mdcore_stub
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Add alias for consistent usage (namespace::target convention)
add_library(mdeditor::mdcore_stub ALIAS mdcore_stub)
