#!/usr/bin/env bash

set -e
set -o pipefail

source ./platform/linux/scripts/setup.sh

BUILDTYPE=${BUILDTYPE:-Release}

make linux -j${JOBS} BUILDTYPE=${BUILDTYPE}
make render -j${JOBS} BUILDTYPE=${BUILDTYPE}
make test -j${JOBS} BUILDTYPE=${BUILDTYPE}
make test-* BUILDTYPE=${BUILDTYPE}
