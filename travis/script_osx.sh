#!/bin/bash

set -x
set -e

mkdir build
cd build
conan install ..
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
macdeployqt bin/MrWriter.app -dmg
echo `curl --upload-file bin/MrWriter.dmg https://transfer.sh/MrWriter.dmg`
