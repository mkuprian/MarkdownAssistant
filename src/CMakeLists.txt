# =============================================================================
# src/ CMakeLists.txt
# Main source directory for mdeditor
# =============================================================================

# Add library and application subdirectories
add_subdirectory(mdcore_stub)
add_subdirectory(mdapp_stub)

# Core libraries
add_subdirectory(gapbuffer)
add_subdirectory(markdown)

# CLI tools (must come after libraries they depend on)
add_subdirectory(cli)

# Qt6 QML application (optional - requires Qt6)
if(MDEDITOR_HAS_QT6)
    add_subdirectory(mdapp)
else()
    message(STATUS "Skipping mdapp (Qt6 not available)")
endif()
