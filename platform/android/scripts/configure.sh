#!/usr/bin/env bash

# inherit core deps
source core_deps.env

# add/override additional deps
LIBPNG_VERSION=1.6.20
LIBJPEG_TURBO_VERSION=1.4.2
SQLITE_VERSION=3.9.1
NUNICODE_VERSION=1.6
LIBZIP_VERSION=0.11.2
JNI_HPP_VERSION=2.0.0

export MASON_ANDROID_ABI=${MASON_PLATFORM_VERSION}