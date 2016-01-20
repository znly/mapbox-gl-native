#!/usr/bin/env bash

set -e
set -o pipefail

# Ensure mason is on the PATH
export PATH="`pwd`/.mason:${PATH}" MASON_DIR="`pwd`/.mason"

BUILDTYPE=${BUILDTYPE:-Release}

export CLANG_TIDY=clang-tidy-3.8

make config
make tidy
