#!/bin/bash

if [[ "$EUID" -ne 0 ]] && [[ "$OSTYPE" != "darwin"* ]]
  then echo "Please run as root! (sudo ./build_all.sh)"
  exit
fi
[[ "$OSTYPE" == "darwin"* ]] && \
  export C_INCLUDE_PATH=$C_INCLUDE_PATH:$(brew --prefix libusb)/include && \
  export LIBRARY_PATH=$LIBRARY_PATH:$(brew --prefix libusb)/lib && \
  export CMAKE_PREFIX_PATH=$(brew --prefix qt@5)
(
cd IMSProg_programmer
rm -rf build/
mkdir build/
cmake -S . -B build/
cmake --build build/ --parallel
cmake --install build/
rm -rf build/
)
(
cd IMSProg_editor
rm -rf build/
mkdir build/
cmake -S . -B build/
cmake --build build/ --parallel
cmake --install build/
rm -rf build/
)
# Reloading the USB rules or creating the app bundles for macOS
[[ "$OSTYPE" != "darwin"* ]] && udevadm control --reload-rules || ./create_macos_appbundles.sh
