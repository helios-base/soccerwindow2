# soccerwindow2
[![GitHub license](https://img.shields.io/github/license/helios-base/soccerwindow2)](https://github.com/helios-base/soccerwindow2/blob/master/LISENCE)


soccerwindow2 is a visualization tool for the RoboCup Soccer Simulator.
- The RoboCup Soccer Simulator: http://github.com/rcsoccersim/
- RoboCup Official Homepage: http://www.robocup.org/


soccerwindow2 works as not only an alternative monitor/logplayer, but also a powerful visual debugger for developing a simulated soccer team.
- Visualizes internal status of agents.
- Provides a formation editor.
- Provides a label editor for machine learning.

## Quick Start

The latest soccerwindow2 depends on the following libraries:
 - Qt5 https://www.qt.io/
 - Boost 1.41 or later https://www.boost.org/
 - The latest librcsc https://github.com/helios-base/librcsc

Please install them first before you start the installation of soccerwindow2.

In the case of Ubuntu 22.04, execute the following commands:

```
sudo apt update
sudo apt install build-essential automake autoconf libtool libboost-all-dev qtbase5-dev qt5-qmake libfontconfig1-dev libaudio-dev libxt-dev libglib2.0-dev libxi-dev libxrender-dev
```

In the case of Ubuntu 16.04 - 20.04, execute the following commands:

```
sudo apt update
sudo apt install build-essential automake autoconf libtool libboost-all-dev qt5-default libfontconfig1-dev libaudio-dev libxt-dev libglib2.0-dev libxi-dev libxrender-dev
```
Then, install librcsc.

soccerwindow2 supports two types of build system.
You can choose either one at your convenience.

### Autotools

From its directory, execute commands:
```
./bootstrap
./configure
make
```
autotools will build the necessary binary as `src/qt/soccerwindow2`.

### CMake

From its directory, execute commands:

```
mkdir build
cd build
cmake ..
make
```
CMake will build the necessary binary as `build/soccerwindow2`.

## Configuring

You may prefer to change the installation directory.
Or, librcsc may be installed to the unusual location.
In these cases, you need to configure the build settings.

### Autotools

The default configuration will set up to install soccerwindow2 components under `/usr/local`.
You may need administrator privileges to install soccerwindow2 into the default location.
This locations can be modified by using configure's `--prefix=DIR` and related options.
See `./configure --help` for more details.
```
./configure --prefix=/path/to/installation
```

In the case of specifying the librcsc location, you need to use `--with-librcsc` option:
```
./configure --with-librcsc=/path/to/librcsc-prefix
```
You may also need to set the environment variable `LD_LIBRARY_PATH`.

### CMake

`ccmake` enables us to modify the build settings.
```
cd build
ccmake ..
```

## Installing

From the root directory(Autotools) or build directory(CMake):
```
make install
```

The default configuration of Autotools will set up to install the soccerwindow2
components in `/usr/local/bin` for the executables.
You may need administrator privilages to install soccerwindow2.
In that case, type `sudo make install`.

## Uninstalling

The following procedures will remove all the files that where installed, but not any directories that were created during the installation process.
You may need to remove `PREFIX/share/soccerwindow2` manually.

### autotools
The soccerwindow2 can be easily removed by entering the distribution directory and running
```
make uninstall
```

### CMake

Find `install_manifest.txt` under the build directory, then execute:
```
xargs rm < install_manifest.txt
```

## Using soccerwindow2

To start soccerwindow2, just type

```
soccerwindow2
```
if you installed the executables in your PATH.

A simple convinient script `sswindow2` will also be installed if you use Autotools.
This script starts soccerwindow2 as a monitor client.
To invoke the script, type
```
sswindow2
```
`sswindow2` is equivalent to invoke `soccerwindow2 -c -d`.

soccerwindow2 will look in your home directory for the configuration file `~/.soccerwindow2`.
If this file does not exist, the new file will be created and populated with
default values.

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
