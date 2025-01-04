#!/bin/bash

export HOME_DIR=$(readlink -f ~)
export CMAKE_INSTALL_BINDIR=$HOME_DIR/.local/bin
export CMAKE_INSTALL_DATAROOTDIR=$HOME_DIR/.imsprog
export UDEVDIR="$CMAKE_INSTALL_DATAROOTDIR"/udev
export LOCAL_APP_DIR=$HOME_DIR/.local/share/applications

[[ "$OSTYPE" == "darwin"* ]] && export C_INCLUDE_PATH=/usr/local/opt/libusb/include
(
cd IMSProg_programmer
rm -rf build/
mkdir build/
cmake -S . -B build/ -DCMAKE_INSTALL_BINDIR=$CMAKE_INSTALL_BINDIR -DCMAKE_INSTALL_DATAROOTDIR=$CMAKE_INSTALL_DATAROOTDIR -DUDEVDIR=$UDEVDIR
cmake --build build/ --parallel
cmake --install build/
rm -rf build/


rm -f $LOCAL_APP_DIR/IMSProg.desktop $LOCAL_APP_DIR/IMSProg_database_update.desktop
cp $CMAKE_INSTALL_DATAROOTDIR/applications/IMSProg.desktop $LOCAL_APP_DIR/IMSProg.desktop
cp $CMAKE_INSTALL_DATAROOTDIR/applications/IMSProg_database_update.desktop $LOCAL_APP_DIR/IMSProg_database_update.desktop

# patch desktop files
sed -i "s|/usr/bin|${CMAKE_INSTALL_BINDIR}|g" "$LOCAL_APP_DIR/IMSProg.desktop"
sed -i "s|/usr/share/pixmaps/IMSProg64.png|${CMAKE_INSTALL_DATAROOTDIR}/pixmaps/IMSProg64.png|g" "$LOCAL_APP_DIR/IMSProg.desktop"

sed -i "s|/usr/bin|${CMAKE_INSTALL_BINDIR}|g" "$LOCAL_APP_DIR/IMSProg_database_update.desktop"
sed -i "s|/usr/share/pixmaps/IMSProg_database_update.png|${CMAKE_INSTALL_DATAROOTDIR}/pixmaps/IMSProg_database_update.png|g" "$LOCAL_APP_DIR/IMSProg_database_update.desktop"
)
(
cd IMSProg_editor
rm -rf build/
mkdir build/
cmake -S . -B build/ -DCMAKE_INSTALL_BINDIR=$CMAKE_INSTALL_BINDIR -DCMAKE_INSTALL_DATAROOTDIR=$CMAKE_INSTALL_DATAROOTDIR
cmake --build build/ --parallel
cmake --install build/
rm -rf build/


rm -f $LOCAL_APP_DIR/IMSProg_editor.desktop
cp $CMAKE_INSTALL_DATAROOTDIR/applications/IMSProg_editor.desktop $LOCAL_APP_DIR/IMSProg_editor.desktop

# patch desktop files
sed -i "s|/usr/bin|${CMAKE_INSTALL_BINDIR}|g" "$LOCAL_APP_DIR/IMSProg_editor.desktop"
sed -i "s|/usr/share/pixmaps/chipEdit64.png|${CMAKE_INSTALL_DATAROOTDIR}/pixmaps/chipEdit64.png|g" "$LOCAL_APP_DIR/IMSProg_editor.desktop"
)

# Reloading the USB rules or creating the app bundles for macOS
[[ "$OSTYPE" != "darwin"* ]] && echo "
Finished building

In order to allow non-root users access to the programmer please run the following commands as root:
cp ${UDEVDIR}/rules.d/71-CH341.rules /usr/lib/udev/rules.d
udevadm control --reload-rules
" || ./create_macos_appbundles.sh
