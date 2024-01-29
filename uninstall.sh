#!/bin/sh

echo "Uninstalling LightlyShaders..."
ORIGINAL_DIR=$(pwd)

cd build
sudo make uninstall

cd $ORIGINAL_DIR