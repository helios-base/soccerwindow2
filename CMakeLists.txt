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

# install destination
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/local" CACHE PATH "Install destination path" FORCE)
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

add_subdirectory(src)

# tests
option(BUILD_TESTING "Build the tests" ON)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

install(FILES xpm/soccerwindow2.xpm
  DESTINATION ${CMAKE_INSTALL_DATADIR}/soccerwindow2
  )
