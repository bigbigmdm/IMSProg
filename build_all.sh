#!/bin/bash
if [ "$EUID" -ne 0 ]
  then echo "Please run as root! (sudo ./build_all.sh)"
  exit
fi
cd IMSProg_programmer
rm -rf build
mkdir build
cd build
cmake ..
make -j4
sudo make install
cd ..
rm -rf build
cd ..
cd IMSProg_editor
rm -rf build
mkdir build
cd build
cmake ..
make -j4
sudo make install
cd ..
rm -rf build
cd ..
