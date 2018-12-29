#!/bin/bash

set -x

source /opt/qt*/bin/qt*-env.sh
qmake CONFIG+=release
make

