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
- Connecting to rcssserver over TCP and WebSocket (2026-11-22):
  - rcssserver (the `develop` branch) accepts monitors over TCP and WebSocket as well as UDP.
    Select the transport by `--transport {udp,tcp,ws}` (default: `udp`) and give the port of the transport by `--port`
    (`server::port` for UDP, `server::tcp_port` for TCP, or `server::ws_port` for WebSocket).
    See "Connecting to rcssserver over UDP, TCP, or WebSocket" in `README.md`.
  - WebSocket uses the request path `/monitor` by default; change it by `--ws-path` (e.g. `/player`).
    It requires Qt WebSockets, which is optional: without it, soccerwindow2 is built without the WebSocket connection.
    Use `-DENABLE_WEBSOCKET=OFF` (CMake) or `./configure --disable-websocket` (Autotools) to build without it explicitly.
  - TCP and WebSocket require the monitor protocol version 3 or later, or -1 (JSON). The versions 1 and 2 are refused before connecting.
  - "Monitor > Connect to ..." opens a new dialog to choose the host, the transport, the port, and the path of WebSocket.
    The choice is kept until soccerwindow2 is closed, and is also used by "Connect" and by starting rcssserver. It is not saved in `~/.soccerwindow2`.
  - When soccerwindow2 starts rcssserver (the launcher dialog or "(Re)start server") for TCP or WebSocket,
    `server::tcp_port=PORT` or `server::ws_port=PORT` is added to the arguments, because these ports are disabled by default.
  - The reason of a connection failure or a disconnection (e.g. `(error no_more_monitor)`, a refused connection, or no message for `--wait-seconds`)
    is shown in the status bar. Over TCP and WebSocket, the disconnection by rcssserver is detected at once,
    so `--auto-quit-mode` quits without waiting for `--wait-seconds`.
  - Fixed: a message received over UDP was not terminated by NUL, and the seconds in the log of the auto quit mode were wrong.
  - Fixed: the CMake build included `config.h` made by an in-tree Autotools build in the top source directory, if it existed.
  - New tests: the transports and the monitor client with fake servers, the "Connect to ..." dialog, and
    `monitor_client_live`, which connects to a real rcssserver when the environment variable `RCSSSERVER` is set (see `docs/build.md`).
- Ubuntu 26.04 and the new build of librcsc (2026-11-23):
  - soccerwindow2 can be built on Ubuntu 26.04 (GCC 15, Qt 5.15.18 / 6.10.2, CMake 4) with both CMake and Autotools.
    Fixed the compile errors with GCC 15 (`<algorithm>` was not included) and the deprecation warning with Qt 6.6 or later (`QColor::setNamedColor()`).
  - CMake build: librcsc is found by `librcscConfig.cmake`, which the CMake build of librcsc installs,
    and by `cmake/FindLibrcsc.cmake` as before if it does not exist (e.g. librcsc installed by Autotools).
    The configuration shows which one is used. See "How librcsc is found" in `docs/build.md`.
  - Autotools build: `configure` looks for librcsc in `~/.local` before `~/local` (the default installation directory of librcsc is now `~/.local`),
    and shows the directory where librcsc is found.
  - Boost is no longer required (it was used only in one place, and librcsc no longer requires it in its header files).
  - CI: Ubuntu 24.04 and 26.04 are tested, the same as librcsc (Ubuntu 22.04 is no longer tested).
    The CMake jobs build librcsc by CMake and check both `librcscConfig.cmake` and `cmake/FindLibrcsc.cmake`.
  - New test: the colors saved in `~/.soccerwindow2` are read back with the alpha channel.
- Faster loading of game logs (2026-11-25):
  - A game log (rcg) is now read into memory at once by `rcsc::gzread_file()` of librcsc and parsed there,
    instead of being read through `rcsc::gzifstream`. This uses the faster rcg parser of librcsc (librcsc PR #123).
  - A gzip file is decompressed by librcsc: with libdeflate if librcsc is built with it, and with zlib otherwise.
    Install `libdeflate-dev` before building librcsc. If librcsc is built with neither, gzip game logs cannot be read.
    See "How game logs are read" in `docs/build.md`.
  - Opening a game log takes about half the time with libdeflate: e.g. about 145 → 67 ms for a JSON game log (53.6 MB decompressed),
    116 → 58 ms for a text game log of version 5, and 93 → 48 ms for version 6.
    With zlib only, it also gets faster (e.g. about 145 → 110 ms for the JSON game log).
    The time printed as `RCG parser elapsed time` (misspelled `RCG paser elapsed time` before) includes reading (and decompressing) the file, as before.
  - librcsc of 2026-11-25 or later (the `develop` branch after PR #123) is required.
    With an older librcsc, the build fails because `rcsc/gz/gzread_file.h` is not found.
  - New test: `rcg_loading` opens small game logs of the text (version 5 and 6) and JSON formats, uncompressed and in gzip.
  - CI: the CMake jobs build librcsc with libdeflate (the test `rcg_loading` reads gzip game logs with it), and the Autotools jobs build it with zlib only, the same as the CI of librcsc.
- Saving the frame shown as an image (2026-11-28):
  - "File > Save Image of This Frame..." (Ctrl+Shift+I) saves the frame shown to a PNG or JPEG file, as large as the field view
    and with the device pixel ratio of the screen (e.g. twice as many pixels on a HiDPI screen with the scale 2).
    The image is drawn when the item is chosen, before the file dialog, so the log may go on playing behind the dialog.
  - The default file name is the name of the game log without `.rcg`/`.rcg.gz` and the cycle (`1234`, or `1234.3` when stopped), e.g. `head_v5-1.png`.
    For the frames received from a server and in the anonymous mode, `soccerwindow2` is used instead of the name of the game log.
    The format is given by the extension (PNG if there is none).
  - The texts in the image are drawn with grayscale antialiasing, while the field view draws them without antialiasing.
    Everything else is the same as the field view, including the debug drawings.
  - This is the first step of the export of images and videos ported from rcssmonitor (`docs/plan/2026-11-28-00_frame-export.md`).
  - New test: `field_canvas_render` checks that the image is the same as the field view, that the frame shown and the state of the view
    do not change after drawing other frames, and the menu item.
- Exporting a range of the frames as images (2026-11-28):
  - "File > Export Images / Video..." (Ctrl+Shift+E) opens a new dialog that exports a range of the frames of the game log as PNG or JPEG files.
    It replaces the old "File > Save Image" dialog.
  - The dialog is modeless: the range can be taken from the position shown while it is open ("Now"), and "Clear" makes it the whole log.
    The start and the end are cycles (`1234`) or stopped frames (`1234.3`, or `1234,3` as the score board shows it).
    An end at a cycle includes its stopped frames (the old dialog did not include them), and a cycle not in the log is moved to the next or the previous frame.
  - The size of the images: as the field view, 1280 x 720, 1920 x 1080, 3840 x 2160, or any.
    "Enlarge the texts, logos and lines with the image" makes the image look as the field view enlarged.
  - The rate: images a second and the speed of the game. The images are named by their numbers from 0 (`image-00000.png`),
    or by the cycles (`image-c01234.00.png`) when an image is a frame.
  - Before the export, the files already there are asked about once (not for each image), and the disk space is estimated.
    A progress dialog shows the time left and can stop the export. The field view does not change.
    The export is not available while connected to a server; the frames kept after disconnecting can be exported.
  - The settings of the dialog are saved in the group `Export` of `~/.soccerwindow2` when an export starts.
    `--image-save-dir`, `--image-name-prefix` and `--image-save-format` given on the command line were used instead of the saved ones for that run
    (replaced by `--export-image-prefix` and `--export-image-format` later; see below).
  - `--auto-image-save` still saved the whole log with the old code at this step (removed later; see below).
  - New tests: `export_timing`, `export_range`, `image_sequence_sink` and `export_dialog`.
- Marking, choosing and exporting several ranges (2026-11-28):
  - "File > Export Images / Video" is now a submenu: "Open the Export Dialog..." (Ctrl+Shift+E), "Start the Range Here" (`[`),
    "End the Range Here" (`]`) and "Clear the Range". The range can be marked at the position shown without opening the dialog.
  - The slider of the log player shows the ranges to export as bands (the current one darker) and the goals as orange ticks,
    also when the tool bar is put at the left or the right of the window.
  - The export dialog has a list of ranges (Add, Remove, Up, Down, and a name of each), and "Presets":
    the whole log, a half (the halves of the extra time and the penalty shootout too), a goal scene (10 s before to 5 s after the goal by default),
    all the goals as a range each, and the frames around the position shown (5 s before and after by default). The seconds are saved.
    The seconds are of the playback: the stopped frames count too, so that 5 s after a goal are the stopped frames before the kick-off
    (a goal scene is 151 frames by default).
    The halves are found by the cycles of the server's parameters in the game log, and the goals are the same as "Go to the next score" uses.
  - Several ranges are exported each in its own directory (by its name), or all in one sequence of images.
  - The ranges belong to the game log open: they are cleared when another log is opened or a server is connected.
  - New tests: `export_range_list` and `log_player_slider`, and more in `export_range` and `export_dialog`.
- Exporting videos by ffmpeg (2026-11-28):
  - The export dialog can write a video instead of the images ("Video (ffmpeg)"): an external ffmpeg, found in the `PATH` or chosen,
    encodes the frames to H.264 by libx264 (libopenh264 or mpeg4 when libx264 is missing, as the dialog tells) in a `.mp4`, `.mov` or `.mkv` file.
    Install ffmpeg to use it (`sudo apt install ffmpeg` on Ubuntu); it is run as a program, and soccerwindow2 is built without it.
  - The dialog has only the quality (CRF, 18 by default) and the encoding speed (preset, `medium` by default).
    The status of ffmpeg (its version and the encoder) is shown when "Video" is chosen.
  - Several ranges are written as a video each (`FILE-NAME.ext`), or all in one video.
    A video stopped is removed; the videos of the ranges already finished are kept. The sizes of a video are made even.
  - "As the Playback" sets the speed of the export to that of the log player.
  - New test: `ffmpeg_sink`, with fake ffmpeg scripts, and with the real ffmpeg if it is installed.
    The CMake Qt6 jobs of the CI install ffmpeg and require the test with the real ffmpeg (`SOCCERWINDOW2_REQUIRE_FFMPEG=1`).
- Exporting from the command line (2026-11-28):
  - `--export-video FILE` or `--export-images DIR` exports a game log without a window and quits,
    e.g. `soccerwindow2 --export-video goals.mp4 --export-range goals game.rcg.gz` writes a video of each goal (`goals-goal1-c1957.mp4`, ...; `goals.mp4` if there is only one).
    No display is needed (soccerwindow2 uses the offscreen platform of Qt for it), e.g. on a server or in a script.
  - The other options are the same as rcssmonitor's: `--export-range RANGE` (`START-END`, `all`, `first-half`, `second-half`, `half-N`,
    `extra-N`, `penalties`, `game`, or `goals[:BEFORE:AFTER]`; given again for more ranges), `--export-join`, `--export-fps`, `--export-speed`,
    `--export-size WxH`, `--export-scale R`, `--export-image-format`, `--export-image-prefix`, `--export-crf`, `--export-preset` and `--ffmpeg PATH`.
    See "Export Options" of `soccerwindow2 --help`.
  - The defaults are fixed values (1280 x 720, 10 images/s, the speed 1, PNG, CRF 18, preset `medium`), not the settings of the export dialog.
    The files already there are overwritten without asking. Nothing is written in `~/.soccerwindow2`
    (the colors and the fonts of the field view are read from it).
  - The progress is told on the standard error. The exit status is 0 when exported, and 1 with the reason (`soccerwindow2: ...`) otherwise.
  - `--export-image-prefix` and `--export-image-format` given without `--export-video` and `--export-images` start soccerwindow2 with the window,
    and the export dialog uses them instead of the saved ones for that run.
  - The options of "Image Save Options" are removed: `--auto-image-save`, `--image-save-dir`, `--image-name-prefix` and `--image-save-format`
    (soccerwindow2 tells them as unsupported options and quits). The old dialog of `--auto-image-save` is removed.
    Instead of `--auto-image-save --image-save-dir DIR --image-name-prefix PREFIX --image-save-format FORMAT`, use
    `--export-images DIR --export-image-prefix PREFIX --export-image-format FORMAT` (PNG or JPEG): the images of the whole game log
    are named by the numbers of the frames as before (`image-00000.png`, ...), 1280 x 720 by default (`--export-size WxH`; the old one used
    the size of the field view; `--export-scale 1` lays out a larger image as the field view of its size, as the old one did).
  - The colors and the fonts of the field view are saved in `~/.soccerwindow2` when the main window is closed (they were saved at the exit of the program).
  - New tests: `export_batch`, and `soccerwindow2_export` (`test/smoke_export.sh`, also run by `make check`), which exports without a display
    and checks that `~/.soccerwindow2` is not written.
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
