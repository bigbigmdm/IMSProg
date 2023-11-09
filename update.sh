#!/bin/bash
wget https://github.com/bigbigmdm/IMSProg/archive/refs/heads/main.zip
unzip main.zip
rm- rf main.zip
cd IMSProg-main
sudo ./build_all.sh
