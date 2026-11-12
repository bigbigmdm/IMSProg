#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)
files=()

if [[ "$OSTYPE" == "darwin"* ]]; then
    CMAKE_INSTALL_PREFIX='/usr/local'
    files+=(
    "/Applications/IMSProg.app"
    "/Applications/IMSProg Database Update.app"
    "/Applications/IMSProg Editor.app")
else
    CMAKE_INSTALL_PREFIX='/usr'
fi

files+=(
    "${CMAKE_INSTALL_PREFIX}/bin/IMSProg"
    "${CMAKE_INSTALL_PREFIX}/share/pixmaps/chipEdit64.png"
    "${CMAKE_INSTALL_PREFIX}/share/pixmaps/IMSProg64.png"
    "${CMAKE_INSTALL_PREFIX}/share/pixmaps/IMSProg_database_update.png"
    "${CMAKE_INSTALL_PREFIX}/share/imsprog"
    "${CMAKE_INSTALL_PREFIX}/share/doc/imsprog"
    "${CMAKE_INSTALL_PREFIX}/bin/IMSProg_database_update"
    "${CMAKE_INSTALL_PREFIX}/share/applications/io.github.bigbigmdm.imsprog.desktop"
    "${CMAKE_INSTALL_PREFIX}/share/applications/io.github.bigbigmdm.imsprog.editor.desktop"
    "${CMAKE_INSTALL_PREFIX}/share/applications/io.github.bigbigmdm.imsprog.database_update.desktop"
    "${CMAKE_INSTALL_PREFIX}/share/man/man1/IMSProg.1.gz"
    "${CMAKE_INSTALL_PREFIX}/share/man/man1/IMSProg_database_update.1.gz"
    "${CMAKE_INSTALL_PREFIX}/share/metainfo/io.github.bigbigmdm.imsprog.metainfo.xml"
    "${CMAKE_INSTALL_PREFIX}/share/metainfo/io.github.bigbigmdm.imsprog.database_update.metainfo.xml"
    "${CMAKE_INSTALL_PREFIX}/bin/IMSProg_editor"
    "${CMAKE_INSTALL_PREFIX}/share/man/man1/IMSProg_editor.1.gz"
    "${CMAKE_INSTALL_PREFIX}/share/metainfo/io.github.bigbigmdm.imsprog.editor.metainfo.xml"
    "${HOME}/.local/share/imsprog")

# Removing hicolor icons (only for Linux)
if [[ "$OSTYPE" != "darwin"* ]]; then
    icon_sizes=("16x16" "22x22" "24x24" "32x32" "48x48" "64x64" "128x128" "256x256" "512x512" "scalable")
    for size in "${icon_sizes[@]}"; do
        apps_dir="${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${size}/apps"
        if [ -d "$apps_dir" ]; then
            while IFS= read -r -d '' icon; do
                files+=("$icon")
            done < <(find "$apps_dir" -maxdepth 1 \( -name "io.github.bigbigmdm.imsprog.*" -o -name "io.github.bigbigmdm.imsprog.editor.*" -o -name "io.github.bigbigmdm.imsprog.database_update.*" \) -print0)
        fi
    done
fi

echo "${bold}Warning: This script will permanently delete the following files and directories:${normal}"

for file in "${files[@]}"; do
        echo "${bold}\"$file\"${normal}"
done

read -rp "Are you sure you want to proceed? (Y/n): " confirm
confirm=${confirm:-y}

if [[ $confirm != "y" && $confirm != "Y" ]]; then
    echo "Operation Canceled."
    exit 1
fi

for file in "${files[@]}"; do
    if [ ! -e "$file" ]; then
        continue
    else
        sudo rm -rf "$file"
    fi
# Linux icon cashe update
if [[ "$OSTYPE" != "darwin"* ]] && command -v gtk-update-icon-cache &> /dev/null; then
    sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor &> /dev/null
fi

done && echo Uninstall Complete
