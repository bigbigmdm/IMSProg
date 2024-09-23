#!/usr/bin/env bash

CONTENTSDIR1='IMSProg.app/Contents'
CONTENTSDIR2='IMSProg Editor.app/Contents'
CONTENTSDIR3='IMSProg Database Update.app/Contents'

create_directories() {
    local CONTENTSDIR=$1
    mkdir -p "$CONTENTSDIR/MacOS"
    mkdir -p "$CONTENTSDIR/Resources"
}

generate_iconset() {
    local INPUT_IMG=$1
    local OUTPUT_ICONS=$2

    mkdir icon.iconset
    sips -z 16 16 -s format png "$INPUT_IMG" --out icon.iconset/icon_16x16.png
    sips -z 32 32 -s format png "$INPUT_IMG" --out icon.iconset/icon_16x16@2x.png
    sips -z 32 32 -s format png "$INPUT_IMG" --out icon.iconset/icon_32x32.png
    sips -z 64 64 -s format png "$INPUT_IMG" --out icon.iconset/icon_32x32@2x.png
    sips -z 128 128 -s format png "$INPUT_IMG" --out icon.iconset/icon_128x128.png
    sips -z 256 256 -s format png "$INPUT_IMG" --out icon.iconset/icon_128x128@2x.png
    sips -z 256 256 -s format png "$INPUT_IMG" --out icon.iconset/icon_256x256.png
    sips -z 512 512 -s format png "$INPUT_IMG" --out icon.iconset/icon_256x256@2x.png

    iconutil -c icns icon.iconset -o "$OUTPUT_ICONS"
    rm -rf icon.iconset
}

# Update Info.plist
update_plist() {
    local CONTENTSDIR=$1
    local ICON_NAME=$2
    local EXECUTABLE_NAME=$3

    /usr/libexec/PlistBuddy -c 'Add :CFBundleDevelopmentRegion string English' "$CONTENTSDIR/Info.plist" 1>/dev/null
    /usr/libexec/PlistBuddy -c "Add :CFBundleIconFile string $ICON_NAME" "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c 'Add :CFBundleInfoDictionaryVersion string 6.0' "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c 'Add :CFBundlePackageType string APPL' "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c 'Add :CFBundleShortVersionString string 0.2' "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c 'Add :CFBundleVersion string 0.2-10' "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c 'Add :NSPrincipalClass string NSApplication' "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c "Add :CFBundleExecutable string $EXECUTABLE_NAME" "$CONTENTSDIR/Info.plist"
    /usr/libexec/PlistBuddy -c 'Add :LSUIElement bool true' "$CONTENTSDIR/Info.plist"
}

# Create app launch script
create_launch_script() {
    local CONTENTSDIR=$1
    local EXECUTABLE_NAME=$2
    local COMMAND=$3

    cat << ENDOFSCRIPT > "$CONTENTSDIR/MacOS/$EXECUTABLE_NAME"
#!/usr/bin/env bash

export PATH=$PATH:/usr/local/bin/IMSProg_editor
bash -c "$COMMAND > /dev/null 2>&1 &"
ENDOFSCRIPT

    chmod u+x "$CONTENTSDIR/MacOS/$EXECUTABLE_NAME"
}

move_app() {
    local APP_NAME=$1
    mv "$APP_NAME" /Applications/"$APP_NAME"
}

create_imsprog_app() {
    create_directories "$CONTENTSDIR1"
    generate_iconset "img/logo_IMSProg.svg" "$CONTENTSDIR1/Resources/IMSProg.icns"
    update_plist "$CONTENTSDIR1" "IMSProg.icns" "IMSProg"
    create_launch_script "$CONTENTSDIR1" "IMSProg" "/usr/local/bin/IMSProg"
    move_app "IMSProg.app"
}

create_imsprog_editor_app() {
    create_directories "$CONTENTSDIR2"
    generate_iconset "IMSProg_editor/img/chipEdit64.png" "$CONTENTSDIR2/Resources/IMSProg_editor.icns"
    update_plist "$CONTENTSDIR2" "IMSProg_editor.icns" "IMSProg_Editor"
    create_launch_script "$CONTENTSDIR2" "IMSProg_Editor" "/usr/local/bin/IMSProg_editor"
    move_app "IMSProg Editor.app"
}

create_imsprog_database_update_app() {
    create_directories "$CONTENTSDIR3"
    generate_iconset "IMSProg_programmer/img/IMSProg_database_update.png" "$CONTENTSDIR3/Resources/IMSProg_database_update.icns"
    update_plist "$CONTENTSDIR3" "IMSProg_database_update.icns" "IMSProg_database_update"
    create_launch_script "$CONTENTSDIR3" "IMSProg_database_update" "/usr/local/bin/IMSProg_database_update"
    move_app "IMSProg Database Update.app"
}

# Run the functions to create the apps
create_imsprog_app > /dev/null
create_imsprog_editor_app > /dev/null
create_imsprog_database_update_app > /dev/null
