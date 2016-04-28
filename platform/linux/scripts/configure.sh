#!/usr/bin/env bash

# inherit core deps
source core_deps.env

# add/override additional deps
BOOST_LIBPROGRAM_OPTIONS_VERSION=1.59.0
GLFW_VERSION=3.1.2
SQLITE_VERSION=3.9.1
LIBCURL_VERSION=system
LIBPNG_VERSION=1.6.20
LIBJPEG_TURBO_VERSION=1.4.2
LIBUV_VERSION=1.7.5
WEBP_VERSION=0.5.0

function print_opengl_flags {
    CONFIG+="    'opengl_cflags%': $(quote_flags $(pkg-config gl x11 --cflags)),"$LN
    CONFIG+="    'opengl_ldflags%': $(quote_flags $(pkg-config gl x11 --libs)),"$LN
}
