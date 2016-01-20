#!/bin/bash

set -e
set -o pipefail

nvm use $NODE_VERSION

./node_modules/.bin/node-pre-gyp package
./node_modules/.bin/node-pre-gyp testpackage
./node_modules/.bin/node-pre-gyp publish info
