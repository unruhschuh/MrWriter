#!/bin/bash

set -x
set -e

mkdir build
cd build
conan install ..
cmake ..
cmake --build . --config Release

