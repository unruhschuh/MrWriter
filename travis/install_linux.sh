#!/bin/bash

set -x

sudo add-apt-repository ppa:beineri/opt-qt-5.15.2-xenial -y
sudo apt-get update -qq
sudo apt-get -y install qt515base libgl1-mesa-dev
source /opt/qt*/bin/qt*-env.sh

