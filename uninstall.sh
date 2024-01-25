#!/bin/sh

ORIGINAL_DIR=$(pwd)

cd build
sudo make uninstall

cd $ORIGINAL_DIR