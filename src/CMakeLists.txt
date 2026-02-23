# =============================================================================
# src/ CMakeLists.txt
# Main source directory for mdeditor
# =============================================================================

# Add library and application subdirectories
add_subdirectory(mdcore_stub)
add_subdirectory(mdapp_stub)

# Core libraries
add_subdirectory(gapbuffer)

# CLI tools
add_subdirectory(cli)
