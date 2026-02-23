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
