#!/bin/bash

set -x

#sudo add-apt-repository ppa:beineri/opt-qt593-trusty -y
#sudo apt-get update -qq
#sudo apt-get -y install qt59base
#source /opt/qt*/bin/qt*-env.sh

#sudo apt-get -y install qt5-default

pip3 install conan
conan user
