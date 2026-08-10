#!/bin/bash
set -e

WORKDIR="$(pwd)"

BUILD_PWD="$WORKDIR/build"
MONITOR_BIN_PATH="$BUILD_PWD/soccerwindow2-bin"
mkdir -p $MONITOR_BIN_PATH
autoreconf -i
automake --add-missing
$WORKDIR/configure --prefix="$MONITOR_BIN_PATH"
make -j$(nproc)
make install
cp $WORKDIR/utils/appimage/soccerwindow2.desktop $BUILD_PWD
cp $WORKDIR/utils/appimage/soccerwindow2.png $BUILD_PWD