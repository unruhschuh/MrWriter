#!/bin/bash

set -x

source /opt/qt*/bin/qt*-env.sh

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

cp -r ../appdir .
make install

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
export VERSION=$(git rev-parse --short HEAD) # linuxdeployqt uses this for naming the file
./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract
PATH=$PATH:./bin ./squashfs-root/AppRun appdir/usr/share/applications/MrWriter.desktop -appimage
APPIMAGE=`ls MrWriter*.AppImage`
curl --upload-file "./${APPIMAGE}" -s -w "\n" http://transfer.sh/${APPIMAGE}

