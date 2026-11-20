# Uninstall.cmake
# ---------------
#
# Remove the files installed by "cmake --install". This script is run by the
# "uninstall" target:
#
#   cmake --build build --target uninstall
#
# The files listed in install_manifest.txt (created in the build directory by
# the install) are removed. The package data directory (a directory named
# PACKAGE_NAME, e.g. share/soccerwindow2) is also removed if it becomes empty.
# The other directories (e.g. bin) are not removed.
#
# Input variables (-D<var>=<value>):
#   MANIFEST      the path of install_manifest.txt
#   PACKAGE_NAME  the package name (e.g. soccerwindow2)

foreach(var MANIFEST PACKAGE_NAME)
  if(NOT ${var})
    message(FATAL_ERROR "Uninstall.cmake: ${var} is not set.")
  endif()
endforeach()

if(NOT EXISTS "${MANIFEST}")
  message(FATAL_ERROR "${MANIFEST} is not found. "
    "The uninstall target removes the files installed from this build directory.")
endif()

file(STRINGS "${MANIFEST}" installed_files)

set(package_dirs)
foreach(file IN LISTS installed_files)
  set(path "$ENV{DESTDIR}${file}")
  if(IS_SYMLINK "${path}" OR EXISTS "${path}")
    message(STATUS "Uninstalling: ${path}")
    file(REMOVE "${path}")
    if(IS_SYMLINK "${path}" OR EXISTS "${path}")
      message(FATAL_ERROR "Failed to remove ${path}")
    endif()
    get_filename_component(dir "${path}" DIRECTORY)
    get_filename_component(dir_name "${dir}" NAME)
    if(dir_name STREQUAL PACKAGE_NAME)
      list(APPEND package_dirs "${dir}")
    endif()
  else()
    message(STATUS "Not found (skipped): ${path}")
  endif()
endforeach()

# Remove the package data directories if they are empty.
if(package_dirs)
  list(REMOVE_DUPLICATES package_dirs)
endif()
foreach(dir IN LISTS package_dirs)
  file(GLOB entries LIST_DIRECTORIES true "${dir}/*" "${dir}/.*")
  list(FILTER entries EXCLUDE REGEX "/\\.\\.?$")
  if(NOT entries)
    message(STATUS "Removing the empty directory: ${dir}")
    file(REMOVE_RECURSE "${dir}")
  endif()
endforeach()
