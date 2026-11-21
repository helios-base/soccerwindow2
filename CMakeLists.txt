cmake_minimum_required(VERSION 3.16)

# Use the BoostConfig.cmake provided by Boost instead of the removed FindBoost module.
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# NOTE: Update the version in configure.ac (AC_INIT) at the same time.
project(soccerwindow2 VERSION 2026 LANGUAGES CXX)

# compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# install destination: ~/.local by default
# (the desktop environments search ~/.local/share for the .desktop file and
# the icons, and ~/.local/bin is in PATH on Ubuntu).
# A build directory configured before keeps its value in the cache.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install destination path" FORCE)
endif()
include(GNUInstallDirs)

# Embed the paths of the linked libraries (e.g. librcsc) into the installed executable.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# librcsc
find_package(Librcsc REQUIRED)

# boost (header-only)
find_package(Boost 1.41 REQUIRED)

# Qt6 or Qt5
set(SOCCERWINDOW2_QT_VERSION "AUTO" CACHE STRING "Qt major version to use (AUTO, 5 or 6). AUTO prefers Qt6.")
set_property(CACHE SOCCERWINDOW2_QT_VERSION PROPERTY STRINGS AUTO 5 6)

if(SOCCERWINDOW2_QT_VERSION STREQUAL "AUTO")
  # Probe Qt6 explicitly. find_package(QT NAMES Qt6 Qt5) may pick Qt5 first
  # when both are installed under the same prefix.
  find_package(Qt6 6.2 QUIET COMPONENTS Core Gui Widgets Network)
  if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
  else()
    set(QT_VERSION_MAJOR 5)
  endif()
elseif(SOCCERWINDOW2_QT_VERSION STREQUAL "5" OR SOCCERWINDOW2_QT_VERSION STREQUAL "6")
  set(QT_VERSION_MAJOR ${SOCCERWINDOW2_QT_VERSION})
else()
  message(FATAL_ERROR "SOCCERWINDOW2_QT_VERSION must be AUTO, 5 or 6: ${SOCCERWINDOW2_QT_VERSION}")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
  set(SOCCERWINDOW2_QT_MIN_VERSION 6.2)
else()
  set(SOCCERWINDOW2_QT_MIN_VERSION 5.15)
endif()
find_package(Qt${QT_VERSION_MAJOR} ${SOCCERWINDOW2_QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Widgets Network)

# zlib
find_package(ZLIB)
if(ZLIB_FOUND)
  set(HAVE_LIBZ TRUE)
endif()

# Qt WebSockets (optional) for the WebSocket connection to rcssserver
option(ENABLE_WEBSOCKET "Enable the WebSocket connection to rcssserver (requires Qt WebSockets)" ON)
set(HAVE_QT_WEBSOCKETS FALSE)
if(ENABLE_WEBSOCKET)
  find_package(Qt${QT_VERSION_MAJOR} ${SOCCERWINDOW2_QT_MIN_VERSION} QUIET COMPONENTS WebSockets)
  if(Qt${QT_VERSION_MAJOR}WebSockets_FOUND)
    set(HAVE_QT_WEBSOCKETS TRUE)
  else()
    message(WARNING "Qt${QT_VERSION_MAJOR} WebSockets not found. The WebSocket connection is disabled.")
  endif()
endif()

# generate config.h
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake-config.h.in
  ${PROJECT_BINARY_DIR}/config.h
  )

# check the settings
message(STATUS "Found librcsc:")
message(STATUS "  Librcsc_LIBRARY=${Librcsc_LIBRARY}")
message(STATUS "  Librcsc_INCLUDE_DIR=${Librcsc_INCLUDE_DIR}")
message(STATUS "Found Qt: ${Qt${QT_VERSION_MAJOR}_VERSION}")
message(STATUS "Build settings:")
message(STATUS "  BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "  INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message(STATUS "  WebSocket connection (Qt WebSockets): ${HAVE_QT_WEBSOCKETS}")

add_subdirectory(src)

# tests
option(BUILD_TESTING "Build the tests" ON)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

# the application icon in the hicolor icon theme of freedesktop.org
# (PREFIX/share/icons/hicolor/SIZExSIZE/apps/soccerwindow2.png and
# PREFIX/share/icons/hicolor/scalable/apps/soccerwindow2.svg)
foreach(size 16 32 48 64 128 256)
  install(FILES icons/app/${size}/soccerwindow2.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps
    )
endforeach()
install(FILES icons/app/svg/soccerwindow2.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
  )

# the desktop entry (PREFIX/share/applications/soccerwindow2.desktop), with
# which the desktop environments can find and start soccerwindow2.
# The directory of the executable is embedded in Exec at the installation,
# so that it is correct even if the prefix is given by "cmake --install
# --prefix DIR". DESTDIR is not included in the path.
if(IS_ABSOLUTE "${CMAKE_INSTALL_BINDIR}")
  set(_sw2_desktop_bindir "${CMAKE_INSTALL_BINDIR}")
else()
  set(_sw2_desktop_bindir "\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
endif()
install(CODE "
  set(SOCCERWINDOW2_BINDIR \"${_sw2_desktop_bindir}\")
  file(READ \"${PROJECT_SOURCE_DIR}/soccerwindow2.desktop.in\" _sw2_desktop)
  string(CONFIGURE \"\${_sw2_desktop}\" _sw2_desktop @ONLY)
  file(WRITE \"${PROJECT_BINARY_DIR}/soccerwindow2.desktop\" \"\${_sw2_desktop}\")
  ")
install(FILES ${PROJECT_BINARY_DIR}/soccerwindow2.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
  )

# uninstall: cmake --build build --target uninstall
# (removes the files listed in install_manifest.txt of the build directory)
add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND}
          -DMANIFEST=${PROJECT_BINARY_DIR}/install_manifest.txt
          -DPACKAGE_NAME=${PROJECT_NAME}
          -P ${PROJECT_SOURCE_DIR}/cmake/Uninstall.cmake
  COMMENT "Uninstalling ${PROJECT_NAME}"
  VERBATIM
  )

# source tarball: cmake --build build --target dist
# (created from the committed files of HEAD by git archive)
set(SOCCERWINDOW2_DIST_DIR "${PROJECT_SOURCE_DIR}" CACHE PATH
  "Directory where the dist target creates the source tarball (default: the top source directory)")
find_package(Git QUIET)
add_custom_target(dist
  COMMAND ${CMAKE_COMMAND}
          -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
          -DOUTPUT_DIR=${SOCCERWINDOW2_DIST_DIR}
          -DPACKAGE_NAME=${PROJECT_NAME}
          -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
          -P ${PROJECT_SOURCE_DIR}/cmake/Dist.cmake
  COMMENT "Creating the source tarball"
  VERBATIM
  )

# check the source tarball: cmake --build build --target distcheck
# (extract, configure, build, test, and install it in <build>/distcheck)
# The lists are passed with '|' because ';' in a custom command is a separator.
string(REPLACE ";" "|" _sw2_librcsc_dirs "${LIBRCSC_INSTALL_DIR}")
string(REPLACE ";" "|" _sw2_prefix_path "${CMAKE_PREFIX_PATH}")
add_custom_target(distcheck
  COMMAND ${CMAKE_COMMAND}
          -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
          -DOUTPUT_DIR=${SOCCERWINDOW2_DIST_DIR}
          -DPACKAGE_NAME=${PROJECT_NAME}
          -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
          -DWORK_DIR=${PROJECT_BINARY_DIR}/distcheck
          -DCTEST_COMMAND=${CMAKE_CTEST_COMMAND}
          -DGENERATOR=${CMAKE_GENERATOR}
          -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
          -DSW2_QT_VERSION=${SOCCERWINDOW2_QT_VERSION}
          -DLIBRCSC_DIRS=${_sw2_librcsc_dirs}
          -DPREFIX_PATH=${_sw2_prefix_path}
          -P ${PROJECT_SOURCE_DIR}/cmake/DistCheck.cmake
  COMMENT "Checking the source tarball"
  VERBATIM
  USES_TERMINAL
  )
