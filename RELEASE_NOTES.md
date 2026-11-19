# Release Notes

soccerwindow2 does not publish versioned releases.
This file summarizes new features and major changes as they are merged.

## Unreleased

- Added this release notes file to track new features and major changes.
- Qt6 support:
  - soccerwindow2 can now be built with Qt6 (6.2 or later) as well as Qt5 (5.15 or later).
  - Qt4 and Qt5 versions older than 5.15 are no longer supported.
  - Font settings saved in `~/.soccerwindow2` by a Qt6 build cannot be read by a Qt5 build.
    If you switch from a Qt6 build back to a Qt5 build, the font settings are reset to the defaults.
- Fixed the column widths of the data view in the formation editor (the intended margins were not applied).
- CMake build:
  - The minimum required CMake version is now 3.16.
  - Qt6 (6.2 or later) is preferred when available; Qt5 (5.15 or later) is used otherwise.
    Use `-DSOCCERWINDOW2_QT_VERSION=5` or `-DSOCCERWINDOW2_QT_VERSION=6` to choose the Qt version explicitly.
  - Boost is now used as header-only; the Boost.System library is no longer required.
  - `LIBRCSC_INSTALL_DIR` and the `LIBRCSC_PREFIX` environment variable are now searched before the system default paths.
  - The installed executable records the library path of librcsc (RPATH), so `LD_LIBRARY_PATH` is no longer needed.
  - Fixed the installation of `sswindow2` from a clean source tree.
- Removed the outdated qmake project files (`*.pro`). Use CMake (recommended) or Autotools to build soccerwindow2.
- Added automated tests in `test/`.
  Run them with `ctest --test-dir build --output-on-failure` (CMake) or `make check` (Autotools).
  The tests use the offscreen platform and a temporary home directory, so no display is needed and `~/.soccerwindow2` is not modified.
  - A smoke test starts soccerwindow2 with `--help` and checks that no signal/slot connection error occurs.
  - A GUI smoke test with Qt Test (CMake only) opens the main window and the other windows and dialogs,
    and checks the signal/slot connections and the main shortcut keys, and that no shortcut key is assigned twice in a window.
  - The tests can be disabled with `-DBUILD_TESTING=OFF` in the CMake build.
- Autotools build:
  - Qt6 (6.2 or later) is now supported. Use `./configure --with-qt=5` or `./configure --with-qt=6` to choose the Qt version (default: auto, Qt6 preferred).
    The qmake and moc commands can also be specified by the `QMAKE` and `QT_MOC` variables.
  - Only the required Qt modules (Core, Gui, Widgets, and Network) are linked.
  - Fixed VPATH builds (building in a directory other than the source directory) and `make distcheck`.
  - The source tarball created by `make dist` now contains the CMake build files.
  - The Boost.System library is no longer required. The minimum Boost version is now 1.41.
