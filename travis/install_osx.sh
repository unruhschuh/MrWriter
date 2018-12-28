#!/bin/bash

set -x

brew update
brew install qt
brew link --force qt
