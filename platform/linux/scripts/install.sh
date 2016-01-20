#!/usr/bin/env bash

set -e
set -o pipefail

sudo apt-get install -yq --no-install-suggests --no-install-recommends \
    clang-3.5 xutils-dev libxxf86vm-dev x11proto-xf86vidmode-dev mesa-utils

git submodule update --init .mason

PATH="`pwd`/.mason:${PATH}" MASON_DIR="`pwd`/.mason" \
mason install mesa 10.4.3
