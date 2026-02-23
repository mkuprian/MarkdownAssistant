# =============================================================================
# gapbuffer - Gap Buffer Text Model Library
# =============================================================================

# Define the static library target
add_library(gapbuffer STATIC)

# Add sources using modern CMake target_sources
target_sources(gapbuffer
    PRIVATE
        gap_buffer.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            gap_buffer.h
)

# Specify C++ standard using target_compile_features (modern CMake idiom)
target_compile_features(gapbuffer
    PUBLIC
        cxx_std_17
)

# Set include directories for consumers
# PUBLIC: available to both this target and consumers
target_include_directories(gapbuffer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Set compiler warnings
target_compile_options(gapbuffer
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Add alias for consistent usage (namespace::target convention)
add_library(mdeditor::gapbuffer ALIAS gapbuffer)
