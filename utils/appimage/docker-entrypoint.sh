#!/bin/sh
set -e

LIBRCSC_REPO="https://github.com/helios-base/librcsc"
LIBRCSC_BRANCH="master"

git clone --depth 1 --branch $LIBRCSC_BRANCH $LIBRCSC_REPO /librcsc

cd /librcsc
./bootstrap
./configure --prefix=/$(whoami)/.local
make -j$(nproc)
make install
cd ..

# if there was ~/.local/lib just put it in the LD_LIBRARY_PATH
if [ -d /$(whoami)/.local/lib ]; then
    echo "/$(whoami)/.local/lib" > /etc/ld.so.conf.d/$(whoami)-local.conf
    ldconfig
fi

cd /soccerwindow2 

bash /soccerwindow2/utils/appimage/build_code.sh
bash /soccerwindow2/utils/appimage/build_appimage.sh