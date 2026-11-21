# Building soccerwindow2

This document describes how to build, test, install, and package soccerwindow2 in detail.
For a quick start, see [README.md](../README.md).

- [Requirements](#requirements)
- [Installing the dependencies on Ubuntu](#installing-the-dependencies-on-ubuntu)
- [Building with CMake (recommended)](#building-with-cmake-recommended)
- [Running the tests](#running-the-tests)
- [Creating the source tarball](#creating-the-source-tarball)
- [Building with Autotools](#building-with-autotools)
- [Starting soccerwindow2 from the desktop](#starting-soccerwindow2-from-the-desktop)
- [The library path of librcsc (RPATH)](#the-library-path-of-librcsc-rpath)
- [Notes](#notes)

## Requirements

| Software | Version | Note |
|---|---|---|
| C++ compiler | C++17 | GCC or Clang |
| Qt | 5.15 or later, or 6.2 or later | Core, Gui, Widgets, Network (and Test for the tests) |
| Qt WebSockets | the same version as Qt | optional, for the WebSocket connection to rcssserver |
| CMake | 3.16 or later | for the CMake build (recommended) |
| Autoconf / Automake | | for the Autotools build |
| Boost | 1.41 or later | only the headers are used |
| librcsc | the latest version | https://github.com/helios-base/librcsc (the `develop` branch is the latest) |
| zlib | | optional |

soccerwindow2 is tested by GitHub Actions on Ubuntu 22.04 (Qt 5.15.3 / 6.2.4) and Ubuntu 24.04 (Qt 5.15.13 / 6.4.2)
with both CMake and Autotools.

## Installing the dependencies on Ubuntu

The following commands are for Ubuntu 22.04 and 24.04.
Install the common packages first:

```
sudo apt update
sudo apt install build-essential cmake autoconf automake libtool libboost-dev zlib1g-dev
```

Then install Qt6 or Qt5 (you can install both and choose one when you build soccerwindow2).

Qt6:
```
sudo apt install qt6-base-dev qt6-base-dev-tools qmake6 libgl-dev libglx-dev libopengl-dev
sudo apt install qt6-websockets-dev      # Ubuntu 24.04 (optional)
sudo apt install libqt6websockets6-dev   # Ubuntu 22.04 (optional)
```

Qt5:
```
sudo apt install qtbase5-dev qtbase5-dev-tools qt5-qmake
sudo apt install libqt5websockets5-dev   # optional
```

`qmake6` / `qt5-qmake` are needed only for the Autotools build.
Qt WebSockets is needed only to connect to rcssserver over WebSocket.
Without it, soccerwindow2 is built without the WebSocket connection (UDP and TCP can be used),
and CMake and configure show a warning.

### librcsc

Build and install librcsc by following its README.
soccerwindow2 looks for librcsc in `~/.local` and `~/local` by default, so it can be installed into either of them.
For example, librcsc can be installed into `~/.local` (or `~/local`) by its Autotools build:

```
sudo apt install libboost-system-dev   # librcsc requires the Boost.System library
git clone -b develop https://github.com/helios-base/librcsc.git
cd librcsc
./bootstrap
./configure --prefix=$HOME/.local    # or --prefix=$HOME/local
make
make install
```

## Building with CMake (recommended)

### cmake-helper.sh

`cmake-helper.sh` in the top directory runs the frequently used CMake commands.
It prints each command before running it, so you can also learn the actual cmake commands.
It can be run from any directory; it always works in the top directory of soccerwindow2.

```
./cmake-helper.sh build        # configure (if needed) and build
./cmake-helper.sh test         # build and run the tests
./cmake-helper.sh run          # build and run soccerwindow2
./cmake-helper.sh run -- -c    # build and run soccerwindow2 with options (e.g. connect to rcssserver)
./cmake-helper.sh install      # build and install (into ~/.local by default)
./cmake-helper.sh uninstall    # remove the installed files
./cmake-helper.sh help         # show the usage
```

The second argument selects the Qt version and the build directory:

| Argument | Qt | Build directory |
|---|---|---|
| `auto` (default) | Qt6 if available, otherwise Qt5 | `build` |
| `qt6` | Qt6 | `build-qt6` |
| `qt5` | Qt5 | `build-qt5` |

For example, `./cmake-helper.sh build qt5` builds soccerwindow2 with Qt5 in `build-qt5`.

All the commands:

| Command | What it does | The main cmake command (for `qt6`) |
|---|---|---|
| `configure [QT]` | (re)configure the build directory | `cmake -S . -B build-qt6 -DSOCCERWINDOW2_QT_VERSION=6` |
| `build [QT]` | configure (if needed) and build | `cmake --build build-qt6 --parallel JOBS` |
| `test [QT]` | build and run the tests | `cd build-qt6 && ctest --output-on-failure` |
| `run [QT] [-- ARGS...]` | build and run soccerwindow2 | `build-qt6/soccerwindow2 ARGS...` |
| `install [QT] [--prefix DIR]` | build and install | `cmake --install build-qt6 [--prefix DIR]` |
| `uninstall [QT]` | remove the files installed from the build directory | `cmake --build build-qt6 --target uninstall` |
| `dist [QT]` | create the source tarball | `cmake --build build-qt6 --target dist` |
| `distcheck [QT]` | create and check the source tarball | `cmake --build build-qt6 --target distcheck` |
| `clean [QT\|all]` | remove the build directory | `rm -rf build-qt6` |
| `help` | show the usage | |

Environment variables:

- `CMAKE_ARGS`: additional options for configure. Configure is run again when it is set. For example:
  ```
  CMAKE_ARGS="-DLIBRCSC_INSTALL_DIR=/opt/librcsc" ./cmake-helper.sh build
  ```
- `JOBS`: the number of parallel build jobs (default: the number of CPUs).

`clean` removes only the build directories that contain `CMakeCache.txt`.

### Using cmake directly

```
cmake -S . -B build
cmake --build build --parallel
cmake --install build
```

The executable is created as `build/soccerwindow2`.

Configure options (`cmake -S . -B build -D<option>=<value>`):

| Option | Default | Description |
|---|---|---|
| `SOCCERWINDOW2_QT_VERSION` | `AUTO` | `AUTO` (Qt6 if available, otherwise Qt5), `5`, or `6` |
| `LIBRCSC_INSTALL_DIR` | `~/.local;~/local` | where librcsc is installed (the environment variable `LIBRCSC_PREFIX` is also searched) |
| `CMAKE_INSTALL_PREFIX` | `~/.local` | the installation directory |
| `CMAKE_BUILD_TYPE` | `Release` | the build type |
| `BUILD_TESTING` | `ON` | build the tests |
| `ENABLE_WEBSOCKET` | `ON` | use Qt WebSockets for the WebSocket connection to rcssserver if it is found (`OFF`: build without it) |
| `SOCCERWINDOW2_DIST_DIR` | the top source directory | where the `dist` target creates the source tarball |

If Qt is installed in an unusual location, add it to `CMAKE_PREFIX_PATH`
(e.g. `-DCMAKE_PREFIX_PATH=/opt/Qt/6.5.3/gcc_64`).

The installed executable records the library path of librcsc (RPATH), so `LD_LIBRARY_PATH` is not needed
(see [The library path of librcsc (RPATH)](#the-library-path-of-librcsc-rpath)).

### Installing and uninstalling

`cmake --install build` (or `./cmake-helper.sh install`) installs the following files:

- `PREFIX/bin/soccerwindow2`
- `PREFIX/bin/sswindow2` (a script that runs `soccerwindow2 -c -d`)
- `PREFIX/share/icons/hicolor/SIZExSIZE/apps/soccerwindow2.png` (SIZE: 16, 32, 48, 64, 128, 256) and
  `PREFIX/share/icons/hicolor/scalable/apps/soccerwindow2.svg`: the application icon in the hicolor icon theme of freedesktop.org
- `PREFIX/share/applications/soccerwindow2.desktop`: the desktop entry, with which the desktop environments can find and start soccerwindow2
  (see [Starting soccerwindow2 from the desktop](#starting-soccerwindow2-from-the-desktop)).
  Its `Exec` has the absolute path of `PREFIX/bin/soccerwindow2`, which is determined at the installation
  (`--prefix` of `cmake --install` is used, and `DESTDIR` is not included).

The default installation directory was `~/local` before. A build directory configured before keeps it in the cache.
To move to `~/.local`, uninstall the files with that build directory, and create the build directory again:

```
./cmake-helper.sh uninstall    # remove the files installed into ~/local
./cmake-helper.sh clean
./cmake-helper.sh install      # install into ~/.local
```

Alternatively, give the installation directory explicitly by `-DCMAKE_INSTALL_PREFIX=$HOME/.local`.

To uninstall, run the `uninstall` target (or `./cmake-helper.sh uninstall`):

```
cmake --build build --target uninstall
```

It removes the files listed in `install_manifest.txt`, which is created in the build directory by the install.
The directories (e.g. `PREFIX/bin` and `PREFIX/share/icons/hicolor/...`) are not removed.
Run it with the same build directory that was used for the install.

## Running the tests

```
./cmake-helper.sh test       # CMake
cd build && ctest --output-on-failure    # CMake (ctest --test-dir build ... with CMake 3.20 or later)
make check                   # Autotools (the smoke test and the icon check)
```

The tests are:

- `soccerwindow2_help` (CMake and Autotools): starts soccerwindow2 with `--help` and checks that no signal/slot connection error occurs.
- `check_icons` (CMake and Autotools): checks the icon files in `icons/` (see [icons.en.md](icons.en.md)). Inkscape is not needed.
- `check_desktop_entry` (CMake and Autotools): checks the desktop entry with `desktop-file-validate` (skipped if it is not installed).
- `gui_smoke` (CMake only, with Qt Test): opens the main window and the other windows and dialogs, and checks the signal/slot connections, the main shortcut keys, the color schemes, the icons, the tool buttons, and the "Connect to ..." dialog.
- `message_splitter`, `monitor_transport`, `monitor_client` (CMake only, with Qt Test): check the connection to rcssserver
  (the transports of UDP, TCP, and WebSocket, and the monitor client) with fake servers in the test programs.
- `connect_dialog` (CMake only, with Qt Test): checks the "Connect to ..." dialog.
- `monitor_client_live` (CMake only, with Qt Test): connects to a real rcssserver over UDP, TCP, and WebSocket
  (see [Testing the connection with rcssserver](#testing-the-connection-with-rcssserver)).
  It is skipped unless the environment variable `RCSSSERVER` is set.

No display is needed: the tests use the offscreen platform.
They also use a temporary home directory, so `~/.soccerwindow2` is neither read nor modified.
To see the windows of the GUI test, run it directly with the `-platform` option:

```
build/test/test_gui_smoke -platform xcb
```

### Testing the connection with rcssserver

`monitor_client_live` starts rcssserver and connects to it with the monitor client of soccerwindow2.
It runs only when the environment variable `RCSSSERVER` is set to the path of rcssserver
(the `develop` branch, which supports TCP and WebSocket), so it is skipped in the CI:

```
RCSSSERVER=/path/to/rcssserver ctest --test-dir build -R monitor_client_live --output-on-failure
```

rcssserver is started with a temporary configuration directory (`RCSS_CONF_DIR`, so `~/.rcssserver` is not used)
and the ports 16000 (UDP), 16100 (TCP), and 16180 (WebSocket), so it does not interfere with another rcssserver.
For each transport and protocol version, the test checks that the client connects, receives the frames,
sends a command, and leaves the server. The WebSocket transport is tested only in a build with Qt WebSockets.

## Creating the source tarball

```
./cmake-helper.sh dist          # or: cmake --build build --target dist
./cmake-helper.sh distcheck     # or: cmake --build build --target distcheck
```

- `dist` creates `soccerwindow2-YYYYMMDD.tar.gz` in the top source directory, where YYYYMMDD is the commit date of HEAD.
  The tarball is extracted to the `soccerwindow2/` directory.
- The tarball is created from the committed files of HEAD by `git archive`, so git and a git working tree are required.
  Uncommitted changes are not included (a warning is shown).
- The development files and the unused files (e.g. the old icons in `icons/legacy/`) are excluded by `export-ignore` in `.gitattributes`.
- `distcheck` creates the tarball, extracts it in `<build directory>/distcheck`, makes the sources read-only,
  and checks that it can be configured, built, tested, installed, and uninstalled (no file must be left).
  The working directory is removed if all the checks pass, and is left for inspection if a check fails.
- The output directory can be changed by `-DSOCCERWINDOW2_DIST_DIR=<dir>`.

The tarball does not contain the files generated by Autotools (e.g. `configure`).
To build it with Autotools, run `./bootstrap` first.

## Building with Autotools

```
./bootstrap
./configure
make
make check
make install
```

The executable is created as `src/qt/soccerwindow2`.
The default installation directory is `/usr/local`; change it by `--prefix=DIR` (e.g. `--prefix=$HOME/.local`, as `reconfig.sh` does).
`make install` installs the same files as the CMake build, including the application icon and the desktop entry.

Configure options and variables:

| Option / variable | Description |
|---|---|
| `--with-qt=auto\|5\|6` | the Qt version (default: `auto`, Qt6 if available) |
| `--with-librcsc=PREFIX` | where librcsc is installed (by default, `~/local`, `~/.local`, `/opt/local`, `/usr/local`, and `/usr` are searched) |
| `--prefix=DIR` | the installation directory |
| `QMAKE=...` | the qmake command to use (e.g. `QMAKE=qmake6`) |
| `QT_MOC=...` | the moc command to use |
| `QT_RCC=...` | the rcc command to use |
| `--disable-rpath` | do not embed the library path of librcsc in the executable (see [The library path of librcsc (RPATH)](#the-library-path-of-librcsc-rpath)) |
| `--disable-websocket` | do not use Qt WebSockets (build without the WebSocket connection to rcssserver). By default, it is used if it is found |

The Qt5 qmake of Ubuntu (`/usr/bin/qmake`) is managed by qtchooser.
If Qt5 is not selected, set `QT_SELECT=qt5`.

`configure` embeds the library path of librcsc in the executable (RPATH) and shows it
(`checking for the library path of librcsc to embed (RPATH)... DIR`), so `LD_LIBRARY_PATH` is not needed.
`make check` runs the tests without adding librcsc to `LD_LIBRARY_PATH` in that case.

Other targets:

- `make uninstall`: uninstall
- `make dist` / `make distcheck`: create / check the source tarball of the Autotools build (`soccerwindow2-2026.tar.gz`)

## Starting soccerwindow2 from the desktop

The desktop environments (e.g. GNOME) find `PREFIX/share/applications/soccerwindow2.desktop`
when `PREFIX/share` is one of the directories that they search:
`~/.local/share` (the default of the CMake build) and the directories in `XDG_DATA_DIRS`
(usually `/usr/local/share` (the default of the Autotools build) and `/usr/share`).

- Search "soccerwindow2" (or "RoboCup") in the Activities of GNOME, and start it.
- Right-click the icon and choose "Connect to the Server" to start it in the same way as `sswindow2` (`soccerwindow2 -c -d`).
- If it does not appear just after the installation, log out and log in again.
- With another installation directory (e.g. `~/local`), the desktop environments do not find it
  unless `PREFIX/share` is added to `XDG_DATA_DIRS`.

`Exec` of the desktop entry has the absolute path of the executable, so `PREFIX/bin` does not need to be in `PATH`.
To start soccerwindow2 by typing `soccerwindow2` in a terminal, `PREFIX/bin` must be in `PATH`.
On Ubuntu, `~/.profile` adds `~/.local/bin` to `PATH` at the login if the directory exists,
so log in again after the first installation into `~/.local`.

## The library path of librcsc (RPATH)

Both the CMake build and the Autotools build embed the library path of librcsc in the executable (RPATH),
so soccerwindow2 starts without `LD_LIBRARY_PATH`, also from the desktop.
Because the path is absolute, build soccerwindow2 again after moving librcsc (e.g. from `~/local` to `~/.local`).

The path is not embedded in the following cases:

- the Autotools build with `--disable-rpath`, or the CMake build with `-DCMAKE_SKIP_INSTALL_RPATH=ON` (e.g. for a package of a distribution),
- librcsc is in a directory that the dynamic linker always searches (e.g. `/usr/lib`).

In the first case, or when librcsc has been moved after building soccerwindow2,
set `LD_LIBRARY_PATH` to the library directory of librcsc, e.g. in `~/.bashrc`:

```
export LD_LIBRARY_PATH=$HOME/.local/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}   # librcsc in ~/.local
export LD_LIBRARY_PATH=$HOME/local/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}    # librcsc in ~/local
```

The desktop environments do not read `~/.bashrc`. To start soccerwindow2 from the desktop, build it with the RPATH.

## Notes

- Color scheme of the widgets:
  soccerwindow2 follows the color scheme of the desktop by default ("View > UI Color Scheme > System").
  Choose "Light" or "Dark" in the same menu to use the light or dark palette regardless of the desktop.
  The selection is saved in `~/.soccerwindow2` and used at the next start.
  The command line option `--ui-color-scheme={system,light,dark}` overrides it only for that run.
  The icons have the images for both the light and dark color schemes (see [icons.en.md](icons.en.md)).
- Connecting to rcssserver over TCP or WebSocket:
  see [README.md](../README.md#connecting-to-rcssserver-over-udp-tcp-or-websocket) for the options (`--transport`, `--port`, `--ws-path`).
  `configure` shows whether the WebSocket connection is built (`checking for QWebSocket... yes`),
  and CMake shows it in the build settings (`WebSocket connection (Qt WebSockets): TRUE`).
- Font settings:
  the font settings saved in `~/.soccerwindow2` by a Qt6 build cannot be read by a Qt5 build.
  If you switch from a Qt6 build back to a Qt5 build, the font settings are reset to the defaults.
- Ubuntu 26.04:
  the current librcsc requires the Boost.System library, which was removed in Boost 1.89, so it cannot be built on Ubuntu 26.04 (Boost 1.90) at the moment.
