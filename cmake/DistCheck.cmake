# DistCheck.cmake
# ---------------
#
# Create the source tarball (see Dist.cmake) and check that it can be built,
# tested, installed, and uninstalled. This script is run by the "distcheck"
# target:
#
#   cmake --build build --target distcheck
#
# Steps:
#   1. create the tarball by Dist.cmake
#   2. extract it in WORK_DIR and make the sources read-only
#   3. configure, build, test (ctest), install it into WORK_DIR, check the
#      desktop entry, start the installed executable, and uninstall it (no
#      file must be left)
#   4. remove WORK_DIR if all the steps succeed
#      (WORK_DIR is left for inspection if a step fails)
#
# Input variables (-D<var>=<value>), in addition to those of Dist.cmake:
#   WORK_DIR        the working directory (removed and recreated)
#   CTEST_COMMAND   the ctest command
#   GENERATOR       the CMake generator
#   CXX_COMPILER    the C++ compiler
#   SW2_QT_VERSION  the value of SOCCERWINDOW2_QT_VERSION
#   LIBRCSC_DIRS    the value of LIBRCSC_INSTALL_DIR ('|' separated)
#   LIBRCSC_CONFIG_DIR      the value of librcsc_DIR if librcscConfig.cmake
#                           is used (empty otherwise)
#   LIBRCSC_DISABLE_CONFIG  the value of CMAKE_DISABLE_FIND_PACKAGE_librcsc
#   PREFIX_PATH     the value of CMAKE_PREFIX_PATH ('|' separated)

foreach(var WORK_DIR CTEST_COMMAND GENERATOR)
  if(NOT ${var})
    message(FATAL_ERROR "DistCheck.cmake: ${var} is not set.")
  endif()
endforeach()

# Guard against removing an unexpected directory.
get_filename_component(work_dir_name "${WORK_DIR}" NAME)
if(NOT work_dir_name STREQUAL "distcheck")
  message(FATAL_ERROR "DistCheck.cmake: WORK_DIR must be a directory named 'distcheck': ${WORK_DIR}")
endif()

find_program(CHMOD_EXECUTABLE chmod)

# Make the directory writable and remove it.
function(sw2_remove_work_dir dir)
  if(EXISTS "${dir}")
    if(CHMOD_EXECUTABLE)
      execute_process(COMMAND ${CHMOD_EXECUTABLE} -R u+w "${dir}")
    endif()
    file(REMOVE_RECURSE "${dir}")
  endif()
endfunction()

# Run a command; stop with an error if it fails.
function(sw2_run step)
  message(STATUS "distcheck: ${step}")
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "distcheck: ${step} failed. "
      "The files are left in ${WORK_DIR} for inspection.")
  endif()
endfunction()

# 1. create the tarball
include(${CMAKE_CURRENT_LIST_DIR}/Dist.cmake)

# 2. extract it and make the sources read-only
sw2_remove_work_dir("${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")
set(src_dir "${WORK_DIR}/${SW2_DIST_DIR_NAME}")
set(build_dir "${WORK_DIR}/build")
set(install_dir "${WORK_DIR}/install")

message(STATUS "distcheck: extract ${SW2_DIST_TARBALL}")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar xzf "${SW2_DIST_TARBALL}"
  WORKING_DIRECTORY "${WORK_DIR}"
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0 OR NOT EXISTS "${src_dir}/CMakeLists.txt")
  message(FATAL_ERROR "distcheck: failed to extract ${SW2_DIST_TARBALL}")
endif()

if(CHMOD_EXECUTABLE)
  execute_process(COMMAND ${CHMOD_EXECUTABLE} -R a-w "${src_dir}")
else()
  message(STATUS "distcheck: chmod is not found. The sources are not made read-only.")
endif()

# 3. configure, build, test, and install
set(configure_args
  -S "${src_dir}"
  -B "${build_dir}"
  -G "${GENERATOR}"
  -DCMAKE_BUILD_TYPE=Release
  -DBUILD_TESTING=ON
  )
if(CXX_COMPILER)
  list(APPEND configure_args "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}")
endif()
if(SW2_QT_VERSION)
  list(APPEND configure_args "-DSOCCERWINDOW2_QT_VERSION=${SW2_QT_VERSION}")
endif()
# use librcsc in the same way as the outer build
if(LIBRCSC_CONFIG_DIR)
  list(APPEND configure_args "-Dlibrcsc_DIR=${LIBRCSC_CONFIG_DIR}")
endif()
if(LIBRCSC_DISABLE_CONFIG)
  list(APPEND configure_args "-DCMAKE_DISABLE_FIND_PACKAGE_librcsc=ON")
endif()
# The values may contain ';'. They are passed as quoted arguments directly to
# execute_process(), because a list variable would split them.
string(REPLACE "|" ";" librcsc_dirs "${LIBRCSC_DIRS}")
string(REPLACE "|" ";" prefix_path "${PREFIX_PATH}")

cmake_host_system_information(RESULT jobs QUERY NUMBER_OF_LOGICAL_CORES)
if(NOT jobs OR jobs LESS 1)
  set(jobs 1)
endif()

message(STATUS "distcheck: configure")
execute_process(
  COMMAND ${CMAKE_COMMAND} ${configure_args}
          "-DLIBRCSC_INSTALL_DIR=${librcsc_dirs}"
          "-DCMAKE_PREFIX_PATH=${prefix_path}"
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "distcheck: configure failed. "
    "The files are left in ${WORK_DIR} for inspection.")
endif()
sw2_run("build" ${CMAKE_COMMAND} --build "${build_dir}" --parallel ${jobs})
message(STATUS "distcheck: test")
execute_process(
  COMMAND ${CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY "${build_dir}"
  RESULT_VARIABLE result
  )
if(NOT result EQUAL 0)
  message(FATAL_ERROR "distcheck: test failed. "
    "The files are left in ${WORK_DIR} for inspection.")
endif()
sw2_run("install" ${CMAKE_COMMAND} --install "${build_dir}" --prefix "${install_dir}")

foreach(file
    bin/soccerwindow2
    bin/sswindow2
    share/icons/hicolor/16x16/apps/soccerwindow2.png
    share/icons/hicolor/256x256/apps/soccerwindow2.png
    share/icons/hicolor/scalable/apps/soccerwindow2.svg
    share/applications/soccerwindow2.desktop)
  if(NOT EXISTS "${install_dir}/${file}")
    message(FATAL_ERROR "distcheck: ${file} is not installed. "
      "The files are left in ${WORK_DIR} for inspection.")
  endif()
endforeach()

# Exec of the desktop entry runs the installed executable.
set(desktop_file "${install_dir}/share/applications/soccerwindow2.desktop")
file(STRINGS "${desktop_file}" desktop_exec REGEX "^Exec=")
set(expected_exec "Exec=\"${install_dir}/bin/soccerwindow2\";Exec=\"${install_dir}/bin/soccerwindow2\" -c -d")
if(NOT desktop_exec STREQUAL expected_exec)
  message(FATAL_ERROR "distcheck: Exec of the desktop entry is wrong: ${desktop_exec} "
    "(expected: ${expected_exec}). The files are left in ${WORK_DIR} for inspection.")
endif()
find_program(DESKTOP_FILE_VALIDATE desktop-file-validate)
if(DESKTOP_FILE_VALIDATE)
  sw2_run("validate the desktop entry" ${DESKTOP_FILE_VALIDATE} "${desktop_file}")
endif()

# The installed executable starts without adding librcsc to LD_LIBRARY_PATH,
# because the library path of librcsc is embedded (RPATH).
sw2_run("start the installed executable" sh "${src_dir}/test/smoke_help.sh" "${install_dir}/bin/soccerwindow2")

sw2_run("uninstall" ${CMAKE_COMMAND} --build "${build_dir}" --target uninstall)
file(GLOB_RECURSE remaining_files "${install_dir}/*")
if(remaining_files)
  string(REPLACE ";" "\n  " remaining_files "${remaining_files}")
  message(FATAL_ERROR "distcheck: the following files are left after uninstall:\n  ${remaining_files}\n"
    "The files are left in ${WORK_DIR} for inspection.")
endif()
if(EXISTS "${install_dir}/share/soccerwindow2")
  message(FATAL_ERROR "distcheck: share/soccerwindow2 is left after uninstall. "
    "The files are left in ${WORK_DIR} for inspection.")
endif()

# 4. clean up
sw2_remove_work_dir("${WORK_DIR}")

message(STATUS "distcheck: ${SW2_DIST_TARBALL} is ready for distribution.")
