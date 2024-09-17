#!/bin/bash

[[ "$OSTYPE" == "darwin"* ]] && export C_INCLUDE_PATH=/usr/local/opt/libusb/include
if [ "$EUID" -ne 0 ]
  then echo "Please run as root! (sudo ./build_all.sh)"
  exit
fi
cd IMSProg_programmer
rm -rf build/
mkdir build/
cmake -S . -B build/
cmake --build build/ --parallel 
cmake --install build/
rm -rf build/
cd .. #IMSProg
cd IMSProg_editor
rm -rf build/
mkdir build/
cmake -S . -B build/
cmake --build build/ --parallel 
cmake --install build/
rm -rf build/
# Reloading the USB rules
[[ "$OSTYPE" != "darwin"* ]] && udevadm control --reload-rules
