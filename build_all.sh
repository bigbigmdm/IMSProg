#!/bin/bash
if [ "$EUID" -ne 0 ]
  then echo "Please run as root! (sudo ./build_all.sh)"
  exit
fi
cd IMSProg_programmer
rm -rf build/
mkdir build/
cmake -S . -B build/
cmake --build build/ --parallel 
sudo cmake --install build/
rm -rf build/
cd .. #IMSProg
cd IMSProg_editor
rm -rf build/
mkdir build/
cmake -S . -B build/
cmake --build build/ --parallel 
sudo cmake --install build/
rm -rf build/
# Reloading the USB rules
sudo udevadm control --reload-rules
