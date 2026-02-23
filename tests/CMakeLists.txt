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
# Test executable: markdown_tests
# -----------------------------------------------------------------------------
add_executable(markdown_tests)

# Add test sources
target_sources(markdown_tests
    PRIVATE
        markdown_tests.cpp
)

# Specify C++ standard
target_compile_features(markdown_tests
    PRIVATE
        cxx_std_17
)

# Link to GoogleTest and markdown library
target_link_libraries(markdown_tests
    PRIVATE
        mdeditor::markdown
        GTest::gtest
        GTest::gtest_main
)

# Set compiler warnings
target_compile_options(markdown_tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# -----------------------------------------------------------------------------
# Test executable: documentcontroller_tests (Qt6 required)
# -----------------------------------------------------------------------------
if(MDEDITOR_HAS_QT6)
    # Find Qt6 Test module
    find_package(Qt6 REQUIRED COMPONENTS Core Test)

    # Enable Qt AUTOMOC for this target
    set(CMAKE_AUTOMOC ON)

    add_executable(documentcontroller_tests)

    # Add test sources
    target_sources(documentcontroller_tests
        PRIVATE
            documentcontroller_tests.cpp
            ${CMAKE_SOURCE_DIR}/src/mdapp/DocumentController.cpp
            ${CMAKE_SOURCE_DIR}/src/mdapp/DocumentController.h
    )

    # Specify C++ standard
    target_compile_features(documentcontroller_tests
        PRIVATE
            cxx_std_17
    )

    # Include directories
    target_include_directories(documentcontroller_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/mdapp
    )

    # Link to GoogleTest, Qt6, and project libraries
    target_link_libraries(documentcontroller_tests
        PRIVATE
            mdeditor::gapbuffer
            mdeditor::markdown
            Qt6::Core
            Qt6::Test
            GTest::gtest
    )

    # Set compiler warnings
    target_compile_options(documentcontroller_tests
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )
endif()

# -----------------------------------------------------------------------------
# Register tests with CTest
# -----------------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(mdtests)
gtest_discover_tests(gapbuffer_tests)
gtest_discover_tests(markdown_tests)

if(MDEDITOR_HAS_QT6)
    # Get Qt bin directory for runtime DLLs
    get_target_property(QT6_CORE_LOCATION Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(NOT QT6_CORE_LOCATION)
        get_target_property(QT6_CORE_LOCATION Qt6::Core IMPORTED_LOCATION_DEBUG)
    endif()
    if(NOT QT6_CORE_LOCATION)
        get_target_property(QT6_CORE_LOCATION Qt6::Core IMPORTED_LOCATION)
    endif()
    if(QT6_CORE_LOCATION)
        get_filename_component(QT6_LIB_DIR "${QT6_CORE_LOCATION}" DIRECTORY)
        # On Windows, Qt6 DLLs are in bin/ not lib/
        if(WIN32)
            get_filename_component(QT6_BIN_DIR "${QT6_LIB_DIR}/../bin" ABSOLUTE)
        else()
            set(QT6_BIN_DIR "${QT6_LIB_DIR}")
        endif()
        message(STATUS "Qt6 bin directory: ${QT6_BIN_DIR}")
    endif()

    # For DocumentController tests, use add_test directly to avoid discovery issues
    # This registers all tests as a single CTest entry
    add_test(
        NAME DocumentControllerTests
        COMMAND documentcontroller_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # Set environment to include Qt DLLs in PATH
    if(WIN32 AND QT6_BIN_DIR)
        set_tests_properties(DocumentControllerTests PROPERTIES
            ENVIRONMENT "PATH=${QT6_BIN_DIR}\;$ENV{PATH}"
        )
    endif()
endif()
