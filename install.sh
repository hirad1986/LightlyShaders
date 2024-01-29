#!/bin/sh

echo "Running updatedb..."
sudo updatedb

echo "Looking for libkwin.so.5 location..."
LIBKWIN_PATH=$(locate libkwin.so.5 | grep '\/libkwin.so.5$')
LIBKWIN_DIR="$(dirname "${LIBKWIN_PATH}")"

if [ ! -f "$LIBKWIN_PATH" ]; then
	echo "libkwin.so.5 library not found, exiting..."
	exit 1
fi

echo "Location of libkwin.so.5 found."

if [ ! -f "$LIBKWIN_DIR/libkwin.so" ]; then
	echo "Creating libkwin.so symlink..."
	sudo ln -s "$LIBKWIN_PATH" "$LIBKWIN_DIR/libkwin.so"
else
	echo "The libkwin.so symlink already in place."
fi

echo "Building LightlyShaders..."

ORIGINAL_DIR=$(pwd)

rm -rf build
mkdir build
cd build

cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)

echo "Installing LightlyShaders..."
sudo make install

cd $ORIGINAL_DIR
