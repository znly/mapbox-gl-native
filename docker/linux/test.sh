#!/usr/bin/env bash

# set -e
# set -o pipefail

export _CXX=g++-5
export _CC=gcc-5
export BUILDTYPE=Release

# Node
. ~/.nvm/nvm.sh
nvm use 4.4.5

cd build

# before_install
source ./scripts/travis_helper.sh
source ./scripts/travis_setup.sh

make linux
make test
