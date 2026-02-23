# =============================================================================
# FindThirdParty.cmake - Placeholder for custom Find modules
# =============================================================================
#
# This file serves as a template/placeholder for adding custom CMake Find
# modules for third-party dependencies.
#
# HOW TO ADD A FIND MODULE:
# -------------------------
# 1. Create a new file named Find<PackageName>.cmake in this cmake/ directory.
#    Example: FindMarkdownParser.cmake
#
# 2. In your Find module, define the following variables:
#    - <PackageName>_FOUND       - True if the package was found
#    - <PackageName>_INCLUDE_DIRS - Include directories for the package
#    - <PackageName>_LIBRARIES   - Libraries to link against
#    - <PackageName>_VERSION     - (Optional) Version of the package
#
# 3. Optionally create an IMPORTED target for modern CMake usage:
#    if(<PackageName>_FOUND AND NOT TARGET <PackageName>::<PackageName>)
#        add_library(<PackageName>::<PackageName> UNKNOWN IMPORTED)
#        set_target_properties(<PackageName>::<PackageName> PROPERTIES
#            IMPORTED_LOCATION "${<PackageName>_LIBRARY}"
#            INTERFACE_INCLUDE_DIRECTORIES "${<PackageName>_INCLUDE_DIR}"
#        )
#    endif()
#
# 4. Use find_package_handle_standard_args() for consistent behavior:
#    include(FindPackageHandleStandardArgs)
#    find_package_handle_standard_args(<PackageName>
#        REQUIRED_VARS <PackageName>_LIBRARY <PackageName>_INCLUDE_DIR
#        VERSION_VAR <PackageName>_VERSION
#    )
#
# EXAMPLE FIND MODULE SKELETON:
# -----------------------------
# # FindMyLib.cmake
# find_path(MyLib_INCLUDE_DIR
#     NAMES mylib.h
#     PATHS /usr/local/include /opt/mylib/include
# )
#
# find_library(MyLib_LIBRARY
#     NAMES mylib
#     PATHS /usr/local/lib /opt/mylib/lib
# )
#
# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(MyLib
#     REQUIRED_VARS MyLib_LIBRARY MyLib_INCLUDE_DIR
# )
#
# if(MyLib_FOUND AND NOT TARGET MyLib::MyLib)
#     add_library(MyLib::MyLib UNKNOWN IMPORTED)
#     set_target_properties(MyLib::MyLib PROPERTIES
#         IMPORTED_LOCATION "${MyLib_LIBRARY}"
#         INTERFACE_INCLUDE_DIRECTORIES "${MyLib_INCLUDE_DIR}"
#     )
# endif()
#
# mark_as_advanced(MyLib_INCLUDE_DIR MyLib_LIBRARY)
# =============================================================================

message(STATUS "FindThirdParty.cmake loaded - placeholder for custom Find modules")
