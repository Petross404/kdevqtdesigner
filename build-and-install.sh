#!/bin/sh

mkdir -p build
mkdir -p plugins

BASE_DIR=`pwd`

cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$BASE_DIR/plugins
make
make install
