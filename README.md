# soccerwindow2
[![GitHub license](https://img.shields.io/github/license/helios-base/soccerwindow2)](https://github.com/helios-base/soccerwindow2/blob/master/LICENSE)


soccerwindow2 is a visualization tool for the RoboCup Soccer Simulator.
- The RoboCup Soccer Simulator: http://github.com/rcsoccersim/
- RoboCup Official Homepage: http://www.robocup.org/


soccerwindow2 works as not only an alternative monitor/logplayer, but also a powerful visual debugger for developing a simulated soccer team.
- Visualizes internal status of agents.
- Provides a formation editor.
- Provides a label editor for machine learning.

## Requirements

- Qt 5.15 or later, or Qt 6.2 or later https://www.qt.io/
  - Qt WebSockets is optional. It is needed to connect to rcssserver over WebSocket.
- CMake 3.16 or later (or Autotools)
- The latest librcsc https://github.com/helios-base/librcsc (the `develop` branch is the latest)

## Quick Start

The following commands are for Ubuntu 24.04 and 26.04.
See [docs/build.md](docs/build.md) for the details (Qt5, Autotools, configure options, tests, source tarball, etc.).

Install the packages (Qt6):

```
sudo apt update
sudo apt install build-essential cmake zlib1g-dev libdeflate-dev qt6-base-dev qt6-base-dev-tools libgl-dev libglx-dev libopengl-dev
sudo apt install qt6-websockets-dev      # optional, for WebSocket
```

For Qt5, install `qtbase5-dev qtbase5-dev-tools` (and optionally `libqt5websockets5-dev`) instead of the Qt6 packages.
Without Qt WebSockets, soccerwindow2 is built without the WebSocket connection (UDP and TCP can be used).

Then, install librcsc by following its README (see also [docs/build.md](docs/build.md#librcsc)).
librcsc is installed into `~/.local` by default, where soccerwindow2 looks for it (and also in `~/local`).

Build and install soccerwindow2 with `cmake-helper.sh`:

```
./cmake-helper.sh build
./cmake-helper.sh install
```

`cmake-helper.sh` prints the cmake commands that it runs. The commands above are equivalent to:

```
cmake -S . -B build
cmake --build build --parallel
cmake --install build
```

soccerwindow2 is installed into `~/.local` by default (`~/.local/bin/soccerwindow2`,
and the application icon and the desktop entry in `~/.local/share`).
To uninstall it, run `./cmake-helper.sh uninstall` (or `cmake --build build --target uninstall`).
The default installation directory was `~/local` before. If you have a build directory configured before,
run `./cmake-helper.sh uninstall` and `./cmake-helper.sh clean` first, because it keeps `~/local`
(see [docs/build.md](docs/build.md#installing-and-uninstalling)).
Use `./cmake-helper.sh build qt5` (or `-DSOCCERWINDOW2_QT_VERSION=5`) to build it with Qt5 when both Qt5 and Qt6 are installed.
Run `./cmake-helper.sh help` for the other commands (test, run, dist, clean, etc.).

## Using soccerwindow2

To start soccerwindow2, just type

```
soccerwindow2
```
if you installed the executables in your PATH.
On Ubuntu, `~/.local/bin` is added to PATH at the login if it exists, so log in again after the first installation.

soccerwindow2 can also be started from the desktop: search "soccerwindow2" (or "RoboCup") in the Activities of GNOME.
Right-click the icon and choose "Connect to the Server" to start it as a monitor client (the same as `sswindow2` below).
See [docs/build.md](docs/build.md#starting-soccerwindow2-from-the-desktop) for the details.

A simple convenient script `sswindow2` is also installed.
This script starts soccerwindow2 as a monitor client.
To invoke the script, type
```
sswindow2
```
`sswindow2` is equivalent to invoke `soccerwindow2 -c -d`.

### Connecting to rcssserver over UDP, TCP, or WebSocket

soccerwindow2 connects to rcssserver over UDP by default.
rcssserver (the `develop` branch, since November 2026) also accepts monitors over TCP and WebSocket.
Its TCP and WebSocket ports are disabled by default; enable them by `server::tcp_port` and `server::ws_port`.
Select the transport by `--transport` and give the port number of the transport by `--port`:

```
rcssserver server::tcp_port=6000 server::ws_port=6080

soccerwindow2 -c                                # UDP (server::port, 6000 by default)
soccerwindow2 -c --transport tcp --port 6000    # TCP (server::tcp_port)
soccerwindow2 -c --transport ws --port 6080     # WebSocket (server::ws_port)
```

- `server::tcp_port=-1` uses the same number as `server::port` (UDP and TCP use different port spaces).
- WebSocket uses the request path `/monitor` by default. `--ws-path /player` can also be used.
  The WebSocket transport is available only when soccerwindow2 is built with Qt WebSockets.
- TCP and WebSocket require the monitor protocol version 3 or later, or -1 (JSON) (`--client-version`, 5 by default).
  The binary formats of the versions 1 and 2 can be used only over UDP.
- "Monitor > Connect to ..." opens a dialog to choose the host, the transport, the port, and the path of WebSocket.
  The choice is kept until soccerwindow2 is closed (it is not saved in `~/.soccerwindow2`).
- When soccerwindow2 starts rcssserver ("Monitor > Launcher Dialog" or "(Re)start server"), it adds `server::tcp_port=PORT`
  or `server::ws_port=PORT` to the arguments of rcssserver for TCP or WebSocket.
- Over TCP and WebSocket, the disconnection by rcssserver (e.g. at the end of a game) is detected at once,
  so `--auto-quit-mode` quits without waiting for `--wait-seconds`.
- `sswindow2` passes the options to soccerwindow2, e.g. `sswindow2 --transport tcp --port 6000`.

soccerwindow2 will look in your home directory for the configuration file `~/.soccerwindow2`.
If this file does not exist, the new file will be created and populated with
default values.

soccerwindow2 follows the color scheme of the desktop by default.
To use the light or dark color scheme regardless of the desktop, choose "View > UI Color Scheme > Light" or "Dark".
The selection is saved and used at the next start.
The command line option `--ui-color-scheme={system,light,dark}` overrides it only for that run.

The font settings saved by a Qt6 build cannot be read by a Qt5 build.
If you switch from a Qt6 build back to a Qt5 build, the font settings are reset to the defaults.

The library path of librcsc is embedded in the executable (RPATH) by both CMake and Autotools,
so `LD_LIBRARY_PATH` is usually not needed. It is needed only when soccerwindow2 is built without the RPATH
(`./configure --disable-rpath` or `-DCMAKE_SKIP_INSTALL_RPATH=ON`), or when librcsc has been moved after building soccerwindow2
(build it again in that case). Set it to the library directory of librcsc, e.g. in `~/.bashrc`:

```
export LD_LIBRARY_PATH=$HOME/.local/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}   # librcsc in ~/.local
export LD_LIBRARY_PATH=$HOME/local/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}    # librcsc in ~/local
```

The desktop does not read `~/.bashrc`, so build soccerwindow2 with the RPATH to start it from the desktop
(see [docs/build.md](docs/build.md#the-library-path-of-librcsc-rpath)).

## Reference

- Hidehisa Akiyama, Tomoharu Nakashima, HELIOS Base: An Open Source Package for the RoboCup Soccer 2D Simulation, In Sven Behnke, Manuela
Veloso, Arnoud Visser, and Rong Xiong editors, RoboCup2013: Robot World XVII, Lecture Notes in Artificial Intelligence, Springer Verlag, Berlin, 2014.

```
@inbook{akiyama2014,
  author="Hidehisa AKIYAMA and Tomoharu NAKASHIMA",
  editor="Sven Behnke and Manuela Veloso and Arnoud Visser and Rong Xiong",
  title="HELIOS Base: An Open Source Package for the RoboCup Soccer 2D Simulation",
  booktitle="RoboCup 2013: Robot World Cup XVII",
  series="Lecture Notes in Artificial Intelligence",
  year="2014",
  publisher="Springer",
  address="Berlin",
  pages="528--535",
  isbn="978-3-662-44468-9",
  doi="10.1007/978-3-662-44468-9_46",
  url="http://dx.doi.org/10.1007/978-3-662-44468-9_46"
}
```
