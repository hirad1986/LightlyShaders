#!/bin/sh

LIBKWIN_PATH=$(locate libkwin.so.5 | grep '\/libkwin.so.5$')
LIBKWIN_DIR="$(dirname "${LIBKWIN_PATH}")"

if [ ! -f "$LIBKWIN_PATH" ]; then
	echo "libkwin.so.5 library not found, exiting..."
	exit 1
fi

if [ ! -f "$LIBKWIN_DIR/libkwin.so" ]; then
	echo "Creating libkwin.so symlink..."
	sudo ln -s "$LIBKWIN_PATH" "$LIBKWIN_DIR/libkwin.so"
fi

ORIGINAL_DIR=$(pwd)

rm -rf build
mkdir build
cd build

cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
sudo make install

cd $ORIGINAL_DIR