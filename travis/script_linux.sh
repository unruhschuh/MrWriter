#!/bin/bash

set -x

export CC=gcc-7
export CXX=g++-7

sudo unlink /usr/bin/g++ && sudo ln -s /usr/bin/g++-7 /usr/bin/g++
g++ --version

#source /opt/qt*/bin/qt*-env.sh

set -e
mkdir build
cd build
#qmake CONFIG+=release ..
conan install ..
cmake .. --DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cp -R ../appdir .
make INSTALL_ROOT=appdir install ; find appdir/
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=$(git rev-parse --short HEAD) # linuxdeployqt uses this for naming the file
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage

find appdir -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq
echo `curl --upload-file MrWriter*.AppImage https://transfer.sh/MrWriter-git.$(git rev-parse --short HEAD)-x86_64.AppImage`
# wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
# bash upload.sh MrWriter*.AppImage*


