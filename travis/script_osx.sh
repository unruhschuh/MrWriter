#!/bin/bash

set -x
set -e

#conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
export PATH=/usr/local/opt/qt5/bin:$PATH
mkdir build
cd build
#conan install .. -s compiler=gcc
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
macdeployqt bin/MrWriter.app -dmg
echo `curl --upload-file bin/MrWriter.dmg https://transfer.sh/MrWriter.dmg`
curl --upload-file bin/MrWriter.dmg -s -w "\n" https://transfer.sh/MrWriter.dmg
