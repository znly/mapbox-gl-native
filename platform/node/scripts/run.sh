#!/usr/bin/env bash

set -e
set -o pipefail

source ./platform/${TRAVIS_OS_NAME}/scripts/setup.sh

BUILDTYPE=${BUILDTYPE:-Release}

nvm use $NODE_VERSION
npm install --build-from-source

# Travis OS X has no GPU
if [[ ${TRAVIS_OS_NAME} == "linux" ]]; then
    npm test
    npm run test-suite
fi
