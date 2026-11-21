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
- CMake 3.16 or later (or Autotools)
- Boost 1.41 or later (only the headers are used) https://www.boost.org/
- The latest librcsc https://github.com/helios-base/librcsc (the `develop` branch is the latest)

## Quick Start

The following commands are for Ubuntu 22.04 and 24.04.
See [docs/build.md](docs/build.md) for the details (Qt5, Autotools, configure options, tests, source tarball, etc.).

Install the packages (Qt6):

```
sudo apt update
sudo apt install build-essential cmake libboost-dev zlib1g-dev qt6-base-dev qt6-base-dev-tools libgl-dev libglx-dev libopengl-dev
```

For Qt5, install `qtbase5-dev qtbase5-dev-tools` instead of the Qt6 packages.

Then, install librcsc into `~/local` by following its README
(see also [docs/build.md](docs/build.md#librcsc)).

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

soccerwindow2 is installed into `~/local` by default (`~/local/bin/soccerwindow2`).
To uninstall it, run `./cmake-helper.sh uninstall` (or `cmake --build build --target uninstall`).
Use `./cmake-helper.sh build qt5` (or `-DSOCCERWINDOW2_QT_VERSION=5`) to build it with Qt5 when both Qt5 and Qt6 are installed.
Run `./cmake-helper.sh help` for the other commands (test, run, dist, clean, etc.).

## Using soccerwindow2

To start soccerwindow2, just type

```
soccerwindow2
```
if you installed the executables in your PATH.

A simple convenient script `sswindow2` is also installed.
This script starts soccerwindow2 as a monitor client.
To invoke the script, type
```
sswindow2
```
`sswindow2` is equivalent to invoke `soccerwindow2 -c -d`.

soccerwindow2 will look in your home directory for the configuration file `~/.soccerwindow2`.
If this file does not exist, the new file will be created and populated with
default values.

soccerwindow2 follows the color scheme of the desktop by default.
To use the light or dark color scheme regardless of the desktop, choose "View > UI Color Scheme > Light" or "Dark".
The selection is saved and used at the next start.
The command line option `--ui-color-scheme={system,light,dark}` overrides it only for that run.

The font settings saved by a Qt6 build cannot be read by a Qt5 build.
If you switch from a Qt6 build back to a Qt5 build, the font settings are reset to the defaults.

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
