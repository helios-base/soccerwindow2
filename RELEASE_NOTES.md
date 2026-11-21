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
  - Choose the color scheme from "View > UI Color Scheme": System (the color scheme of the desktop, default), Light, or Dark.
    The selection is saved in `~/.soccerwindow2` and used at the next start.
  - Light and Dark use the palettes of the Fusion style (Light has the same colors as the Qt5 build), regardless of the desktop and the version of Qt.
    Note that Qt6 follows the dark color scheme of the desktop with System, while a Qt5 build usually does not.
  - The command line option `--ui-color-scheme {system,light,dark}` overrides the saved selection for that run only.
  - The checked buttons in the tool bars are drawn with a translucent background and a border of the highlight color, so that the checked state can be seen clearly with both light and dark color schemes.
- New icons:
  - All the icons of the tool bars and the menus are redrawn with the same line widths and colors.
    Each icon has the images for the light and dark color schemes in 16px and 32px (for high-DPI displays),
    and the images are switched automatically when the color scheme is changed.
  - Some icons are redesigned so that their functions can be understood from the icons,
    e.g. Insert Data, Replace Data, and ReverseY of the formation editor, the debug server, and opening a game log.
  - The number icons of the debug levels are drawn at run time.
  - New application icons: soccerwindow2 (a soccer ball in front of the field of the formation editor) and the formation editor.
    The window icons have the images of 16px to 256px.
  - The icons are drawn as SVG files in `icons/` and converted to PNG files by `icons/generate.sh` with Inkscape.
    The PNG files are committed and embedded with the Qt resource system, so Inkscape is needed only to change the icons.
    See `docs/icons.md` (Japanese) or `docs/icons.en.md` (English) to add or modify an icon.
  - The old XPM icons are kept in `icons/legacy/` as a record. They are not used by the build and are not included in the source tarball.
- Installation: the application icon is installed in the hicolor icon theme of freedesktop.org
  (`PREFIX/share/icons/hicolor/SIZExSIZE/apps/soccerwindow2.png` and `PREFIX/share/icons/hicolor/scalable/apps/soccerwindow2.svg`)
  instead of `PREFIX/share/soccerwindow2/soccerwindow2.xpm`.
- Starting from the desktop: the desktop entry `PREFIX/share/applications/soccerwindow2.desktop` is installed,
  so that soccerwindow2 can be found and started from the desktop environments (e.g. search "soccerwindow2" in the Activities of GNOME).
  Its context menu has "Connect to the Server", which starts soccerwindow2 in the same way as `sswindow2` (`soccerwindow2 -c -d`).
  `Exec` of the desktop entry has the absolute path of the installed executable.
- Fixed the column widths of the data view in the formation editor (the intended margins were not applied).
- CMake build:
  - The minimum required CMake version is now 3.16.
  - Qt6 (6.2 or later) is preferred when available; Qt5 (5.15 or later) is used otherwise.
    Use `-DSOCCERWINDOW2_QT_VERSION=5` or `-DSOCCERWINDOW2_QT_VERSION=6` to choose the Qt version explicitly.
  - Boost is now used as header-only; the Boost.System library is no longer required.
  - `LIBRCSC_INSTALL_DIR` and the `LIBRCSC_PREFIX` environment variable are now searched before the system default paths.
  - The installed executable records the library path of librcsc (RPATH), so `LD_LIBRARY_PATH` is no longer needed.
  - Fixed the installation of `sswindow2` from a clean source tree.
  - The default installation directory is changed from `~/local` to `~/.local`, which the desktop environments search for the desktop entry and the icons.
    A build directory configured before keeps `~/local` in the cache. To move to `~/.local`, run `./cmake-helper.sh uninstall` (to remove the files in `~/local`)
    and `./cmake-helper.sh clean` before installing, or give `-DCMAKE_INSTALL_PREFIX=$HOME/.local`.
  - Added the `uninstall` target (`cmake --build build --target uninstall`), which removes the files listed in `install_manifest.txt`.
- Removed the outdated qmake project files (`*.pro`). Use CMake (recommended) or Autotools to build soccerwindow2.
- Added automated tests in `test/`.
  Run them with `./cmake-helper.sh test` or `cd build && ctest --output-on-failure` (CMake), or `make check` (Autotools).
  The tests use the offscreen platform and a temporary home directory, so no display is needed and `~/.soccerwindow2` is not modified.
  - A smoke test starts soccerwindow2 with `--help` and checks that no signal/slot connection error occurs.
  - A GUI smoke test with Qt Test (CMake only) opens the main window and the other windows and dialogs,
    and checks the signal/slot connections, the main shortcut keys (no shortcut key is assigned twice in a window),
    the color schemes, the icons, and the checked tool buttons.
  - An icon check (`check_icons`, CMake and Autotools) checks the icon files in `icons/` without Inkscape.
  - A desktop entry check (`check_desktop_entry`, CMake and Autotools) checks the desktop entry with `desktop-file-validate` (skipped if it is not installed).
  - The tests can be disabled with `-DBUILD_TESTING=OFF` in the CMake build.
- Source tarball:
  - `cmake --build build --target dist` creates `soccerwindow2-YYYYMMDD.tar.gz` (YYYYMMDD is the commit date of HEAD) from the committed files by `git archive`.
    The tarball is created in the top source directory (change it with `-DSOCCERWINDOW2_DIST_DIR=<dir>`) and is extracted to the `soccerwindow2/` directory.
    Uncommitted changes are not included. The development files and the unused files are excluded by `export-ignore` in `.gitattributes`.
  - `cmake --build build --target distcheck` creates the tarball and checks that it can be configured, built, tested, and installed in `build/distcheck` (the same as `make distcheck` of the Autotools build). It also checks the installed desktop entry, that the installed executable starts without `LD_LIBRARY_PATH`, and that the uninstall leaves no file.
- Documentation: `README.md` now has a short quick start for CMake, and the details of building, testing, installing, and packaging are described in `docs/build.md`.
- Added the GitHub Actions workflow (`.github/workflows/build.yml`).
  It builds soccerwindow2 with Qt5 and Qt6 on Ubuntu 22.04 and 24.04 by both CMake and Autotools, and runs the tests and the source tarball checks.
- Added `cmake-helper.sh`, shortcuts for the frequently used CMake commands.
  For example, `./cmake-helper.sh build qt6`, `./cmake-helper.sh test`, `./cmake-helper.sh run -- -c`, and `./cmake-helper.sh dist`.
  The actual cmake commands are printed before they are run. Run `./cmake-helper.sh help` for the usage.
- Autotools build:
  - Qt6 (6.2 or later) is now supported. Use `./configure --with-qt=5` or `./configure --with-qt=6` to choose the Qt version (default: auto, Qt6 preferred).
    The qmake, moc, and rcc commands can also be specified by the `QMAKE`, `QT_MOC`, and `QT_RCC` variables.
  - Only the required Qt modules (Core, Gui, Widgets, and Network) are linked.
  - Fixed VPATH builds (building in a directory other than the source directory) and `make distcheck`.
  - The source tarball created by `make dist` now contains the CMake build files.
  - The Boost.System library is no longer required. The minimum Boost version is now 1.41.
  - The library path of librcsc is embedded in the executable (RPATH), so `LD_LIBRARY_PATH` is no longer needed, also when starting from the desktop.
    It is not embedded with `--disable-rpath` or when librcsc is in a directory that the dynamic linker always searches (e.g. `/usr/lib`).
    `make check` runs the tests without adding librcsc to `LD_LIBRARY_PATH` when the path is embedded.
  - `reconfig.sh` installs soccerwindow2 into `~/.local` (it was `~/local`).
