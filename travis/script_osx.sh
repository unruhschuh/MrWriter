#!/bin/bash

set -x
set -e

#qmake CONFIG+=release
mkdir build
cd build
conan install ..
cmake .. -DCMAKE_BUILD_TYPE=Release
make ..
macdeployqt bin/MrWriter.app -dmg
echo `curl --upload-file bin/MrWriter.dmg https://transfer.sh/MrWriter.dmg`
