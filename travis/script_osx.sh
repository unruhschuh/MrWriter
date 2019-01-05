#!/bin/bash

set -x

qmake CONFIG+=release
make
macdeployqt MrWriter.app -dmg
curl --upload-file MrWriter.dmg https://transfer.sh/MrWriter.dmg && echo ""
