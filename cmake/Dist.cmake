# Dist.cmake
# ----------
#
# Create the source tarball from the committed files of HEAD by "git archive".
# This script is run by the "dist" target:
#
#   cmake --build build --target dist
#
# The tarball is named <PACKAGE_NAME>-YYYYMMDD.tar.gz, where YYYYMMDD is the
# commit date of HEAD, and it is extracted to <PACKAGE_NAME>/.
# The files with the "export-ignore" attribute in .gitattributes are excluded.
# Uncommitted changes are not included.
#
# Input variables (-D<var>=<value>):
#   SOURCE_DIR     the top source directory (a git working tree)
#   OUTPUT_DIR     the directory where the tarball is created
#   PACKAGE_NAME   the package name (e.g. soccerwindow2)
#   GIT_EXECUTABLE the git command
#
# Output variable (when this script is included by another script):
#   SW2_DIST_TARBALL  the path of the created tarball
#   SW2_DIST_DIR_NAME the name of the top directory in the tarball

foreach(var SOURCE_DIR OUTPUT_DIR PACKAGE_NAME)
  if(NOT ${var})
    message(FATAL_ERROR "Dist.cmake: ${var} is not set.")
  endif()
endforeach()

if(NOT GIT_EXECUTABLE)
  message(FATAL_ERROR "git is not found. The dist target requires git.")
endif()

# The source directory must be the top of a git working tree.
# (If the sources extracted from a tarball are placed in another git working
# tree, "git archive" would archive the other repository.)
execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
  WORKING_DIRECTORY ${SOURCE_DIR}
  RESULT_VARIABLE result
  OUTPUT_VARIABLE toplevel
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  )
get_filename_component(source_dir_real "${SOURCE_DIR}" REALPATH)
if(toplevel)
  get_filename_component(toplevel "${toplevel}" REALPATH)
endif()
if(NOT result EQUAL 0 OR NOT toplevel STREQUAL source_dir_real)
  message(FATAL_ERROR "${SOURCE_DIR} is not the top of a git working tree. "
    "The dist target creates the tarball from the git repository of soccerwindow2.")
endif()

# the commit date of HEAD
execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=format:%Y%m%d HEAD
  WORKING_DIRECTORY ${SOURCE_DIR}
  RESULT_VARIABLE result
  OUTPUT_VARIABLE commit_date
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
if(NOT result EQUAL 0 OR NOT commit_date)
  message(FATAL_ERROR "Failed to get the commit date of HEAD.")
endif()

# warn about uncommitted changes
execute_process(
  COMMAND ${GIT_EXECUTABLE} status --porcelain --untracked-files=no
  WORKING_DIRECTORY ${SOURCE_DIR}
  OUTPUT_VARIABLE changes
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
if(changes)
  message(WARNING "There are uncommitted changes. "
    "The tarball is created from HEAD and does not include them:\n${changes}")
endif()

# The file name has the date, but the top directory does not.
set(SW2_DIST_DIR_NAME "${PACKAGE_NAME}")
set(SW2_DIST_TARBALL "${OUTPUT_DIR}/${PACKAGE_NAME}-${commit_date}.tar.gz")

file(MAKE_DIRECTORY ${OUTPUT_DIR})
execute_process(
  COMMAND ${GIT_EXECUTABLE} archive --format=tar.gz
          --prefix=${SW2_DIST_DIR_NAME}/
          --output=${SW2_DIST_TARBALL}
          HEAD
  WORKING_DIRECTORY ${SOURCE_DIR}
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "git archive failed.")
endif()

message(STATUS "Created ${SW2_DIST_TARBALL}")
