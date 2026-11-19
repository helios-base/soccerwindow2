# FindLibrcsc
# -----------
#
# Find the librcsc library.
#
# Search hints (searched before the system default paths):
#   LIBRCSC_INSTALL_DIR  CMake cache variable (default: $HOME/.local;$HOME/local)
#   LIBRCSC_PREFIX       environment variable
#
# Result variables:
#   Librcsc_FOUND
#   Librcsc_LIBRARY
#   Librcsc_INCLUDE_DIR
#
# Imported target:
#   Librcsc::Librcsc

set(LIBRCSC_INSTALL_DIR "$ENV{HOME}/.local;$ENV{HOME}/local" CACHE PATH "The path where librcsc is installed.")

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

if(Librcsc_FOUND AND NOT TARGET Librcsc::Librcsc)
  add_library(Librcsc::Librcsc UNKNOWN IMPORTED)
  set_target_properties(Librcsc::Librcsc PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "${Librcsc_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Librcsc_INCLUDE_DIR}"
    )
endif()
