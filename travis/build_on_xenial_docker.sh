#!/bin/bash

# runme usind
# docker run -i ubuntu:xenial bash < build_on_xenial_docker.sh

set -x
set -e

apt-get update

# get the newest cmake, because, why not?
apt-get install -y apt-transport-https ca-certificates gnupg software-properties-common wget curl
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main'
apt-get update

# install pip for conan
apt-get install -y git python3-pip libgl1-mesa-dev pkg-config sudo cmake
# apt install libx11-xcb-dev libxcb-render0-dev libxcb-render-util0-dev libxcb-xkb-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xinerama0-dev xkb-data libxcb-dri3-dev libxcb-util-dev
pip3 install conan
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
git clone https://github.com/unruhschuh/MrWriter.git
cd MrWriter
git checkout cmake
mkdir build
cd build
conan install ..
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

