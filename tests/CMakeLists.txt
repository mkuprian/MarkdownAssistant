# =============================================================================
# tests/ CMakeLists.txt
# Unit testing configuration using GoogleTest
# =============================================================================

include(FetchContent)

# -----------------------------------------------------------------------------
# Fetch GoogleTest
# -----------------------------------------------------------------------------
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Prevent GoogleTest from overriding compiler/linker options (Windows)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Download and make available
FetchContent_MakeAvailable(googletest)

# -----------------------------------------------------------------------------
# Test executable: mdtests (stub/core tests)
# -----------------------------------------------------------------------------
add_executable(mdtests)

# Add test sources
target_sources(mdtests
    PRIVATE
        test_stub.cpp
)

# Specify C++ standard
target_compile_features(mdtests
    PRIVATE
        cxx_std_17
)

# Link to GoogleTest and the library under test
target_link_libraries(mdtests
    PRIVATE
        mdeditor::mdcore_stub
        GTest::gtest
        GTest::gtest_main
)

# Set compiler warnings
target_compile_options(mdtests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# -----------------------------------------------------------------------------
# Test executable: gapbuffer_tests
# -----------------------------------------------------------------------------
add_executable(gapbuffer_tests)

# Add test sources
target_sources(gapbuffer_tests
    PRIVATE
        gapbuffer_tests.cpp
)

# Specify C++ standard
target_compile_features(gapbuffer_tests
    PRIVATE
        cxx_std_17
)

# Link to GoogleTest and gapbuffer library
target_link_libraries(gapbuffer_tests
    PRIVATE
        mdeditor::gapbuffer
        GTest::gtest
        GTest::gtest_main
)

# Set compiler warnings
target_compile_options(gapbuffer_tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# -----------------------------------------------------------------------------
# Register tests with CTest
# -----------------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(mdtests)
gtest_discover_tests(gapbuffer_tests)
