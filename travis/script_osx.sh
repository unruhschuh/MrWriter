#!/bin/bash

set -x
set -e

qmake CONFIG+=release
make
macdeployqt MrWriter.app -dmg
echo `curl --upload-file MrWriter.dmg https://transfer.sh/MrWriter.dmg`
