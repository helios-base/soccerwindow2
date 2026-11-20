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
- Color scheme of the widgets:
  - The light palette (the same colors as the Qt5 build) is used by default, even if the desktop uses a dark color scheme.
    Qt6 follows the dark color scheme of the desktop, but the icons are designed for a light background.
  - To follow the color scheme of the desktop, choose "View > UI Color Scheme > System".
    The selection is saved in `~/.soccerwindow2`.
  - The command line option `--ui-color-scheme {light,system}` overrides the saved selection for that run only.
  - With a dark color scheme (System), the black and dark gray lines of the icons are drawn in the text color, so that the icons can be seen on a dark background.
  - The checked buttons in the tool bars are drawn with a translucent background and a border of the highlight color, so that the checked state can be seen clearly with both light and dark color schemes.
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
- Source tarball:
  - `cmake --build build --target dist` creates `soccerwindow2-YYYYMMDD.tar.gz` (YYYYMMDD is the commit date of HEAD) from the committed files by `git archive`.
    The tarball is created in the top source directory (change it with `-DSOCCERWINDOW2_DIST_DIR=<dir>`) and is extracted to the `soccerwindow2/` directory.
    Uncommitted changes are not included. The development files and the unused files are excluded by `export-ignore` in `.gitattributes`.
  - `cmake --build build --target distcheck` creates the tarball and checks that it can be configured, built, tested, and installed in `build/distcheck` (the same as `make distcheck` of the Autotools build).
- Added `cmake-helper.sh`, shortcuts for the frequently used CMake commands.
  For example, `./cmake-helper.sh build qt6`, `./cmake-helper.sh test`, `./cmake-helper.sh run -- -c`, and `./cmake-helper.sh dist`.
  The actual cmake commands are printed before they are run. Run `./cmake-helper.sh help` for the usage.
- Autotools build:
  - Qt6 (6.2 or later) is now supported. Use `./configure --with-qt=5` or `./configure --with-qt=6` to choose the Qt version (default: auto, Qt6 preferred).
    The qmake and moc commands can also be specified by the `QMAKE` and `QT_MOC` variables.
  - Only the required Qt modules (Core, Gui, Widgets, and Network) are linked.
  - Fixed VPATH builds (building in a directory other than the source directory) and `make distcheck`.
  - The source tarball created by `make dist` now contains the CMake build files.
  - The Boost.System library is no longer required. The minimum Boost version is now 1.41.
