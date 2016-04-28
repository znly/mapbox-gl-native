#!/usr/bin/env bash

# inherit core deps
source core_deps.env

# add/override additional deps
BOOST_LIBPROGRAM_OPTIONS_VERSION=1.59.0
LIBCURL_VERSION=system
GLFW_VERSION=3.1.2
LIBPNG_VERSION=1.6.20
LIBJPEG_TURBO_VERSION=1.4.2
SQLITE_VERSION=3.9.1
LIBUV_VERSION=1.7.5
NUNICODE_VERSION=1.6
GTEST_VERSION=1.7.0
PIXELMATCH_VERSION=0.9.0
WEBP_VERSION=0.5.0

function print_opengl_flags {
    CONFIG+="    'opengl_cflags%': $(quote_flags $(pkg-config gl x11 --cflags)),"$LN
    CONFIG+="    'opengl_ldflags%': $(quote_flags $(pkg-config gl x11 --libs)),"$LN
}
