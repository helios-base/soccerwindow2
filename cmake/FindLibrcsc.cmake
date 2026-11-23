# FindLibrcsc
# -----------
#
# Find the librcsc library without librcscConfig.cmake, e.g. librcsc installed
# by its Autotools build. (The CMake build of librcsc installs
# librcscConfig.cmake, which CMakeLists.txt looks for first.)
#
# Search hints (searched before the system default paths):
#   LIBRCSC_INSTALL_DIR  CMake cache variable (defined in CMakeLists.txt)
#   LIBRCSC_PREFIX       environment variable
#
# Result variables:
#   Librcsc_FOUND
#   Librcsc_LIBRARY
#   Librcsc_INCLUDE_DIR
#
# Imported target:
#   librcsc::rcsc  the same name as the imported target of librcscConfig.cmake

find_path(Librcsc_INCLUDE_DIR
  NAMES rcsc/types.h
  HINTS ${LIBRCSC_INSTALL_DIR} ENV LIBRCSC_PREFIX
  PATH_SUFFIXES include
  )

find_library(Librcsc_LIBRARY
  NAMES rcsc
  HINTS ${LIBRCSC_INSTALL_DIR} ENV LIBRCSC_PREFIX
  PATH_SUFFIXES lib
  )

mark_as_advanced(
  Librcsc_INCLUDE_DIR
  Librcsc_LIBRARY
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Librcsc
  REQUIRED_VARS Librcsc_LIBRARY Librcsc_INCLUDE_DIR
  )

if(Librcsc_FOUND AND NOT TARGET librcsc::rcsc)
  add_library(librcsc::rcsc UNKNOWN IMPORTED)
  set_target_properties(librcsc::rcsc PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "${Librcsc_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Librcsc_INCLUDE_DIR}"
    INTERFACE_COMPILE_FEATURES "cxx_std_17"
    )
endif()
