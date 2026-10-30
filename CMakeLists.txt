cmake_minimum_required(VERSION 3.10.0)
project(IMSProg LANGUAGES CXX C)

if(DEFINED ENV{MACPORTS_PORTFILE_DIR} OR DEFINED ENV{PORTNAME})
    set(BUILD_ON_MACPORTS TRUE)
else()
    if(CMAKE_INSTALL_PREFIX MATCHES "^/opt/local" OR CMAKE_PREFIX_PATH MATCHES "/opt/local")
        set(BUILD_ON_MACPORTS TRUE)
    else()
        set(BUILD_ON_MACPORTS FALSE)
    endif()
endif()

if(BUILD_ON_MACPORTS)
    add_compile_definitions(BUILD_ON_MACPORTS=1)
endif()

function(configure_macos_bundle target_name target_icon)
    message(STATUS "Configuring macOS Application Bundle ${target_name}.app on Darwin")
    
    # Define paths for the source image and the generated icon file
    set(ICON_IMG_SOURCE "${PROJECT_ROOT}/${target_icon}")
    message(STATUS "PROJECT_ROOT=${PROJECT_ROOT}/${target_icon}")
    set(ICONSET_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.iconset")
    set(TARGET_ICNS "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.icns")

    # Custom command to generate .icns from SVG/PNG using native macOS tools (sips & iconutil)
    add_custom_command(
        OUTPUT "${TARGET_ICNS}"
        DEPENDS "${ICON_IMG_SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ICONSET_DIR}"
        COMMAND sips -z 16 16     -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_16x16.png"
        COMMAND sips -z 32 32     -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_16x16@2x.png"
        COMMAND sips -z 32 32     -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_32x32.png"
        COMMAND sips -z 64 64     -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_32x32@2x.png"
        COMMAND sips -z 128 128   -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_128x128.png"
        COMMAND sips -z 256 256   -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_128x128@2x.png"
        COMMAND sips -z 256 256   -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_256x256.png"
        COMMAND sips -z 512 512   -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_256x256@2x.png"
        COMMAND sips -z 512 512   -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_512x512.png"
        COMMAND sips -z 1024 1024 -s format png "${ICON_IMG_SOURCE}" --out "${ICONSET_DIR}/icon_512x512@2x.png"
        COMMAND iconutil -c icns "${ICONSET_DIR}" -o "${TARGET_ICNS}"
        COMMENT "Generating macOS .icns icon for ${PROJECT_NAME}..."
    )

    # Set target properties for the macOS application bundle
    set_target_properties(${target_name} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${target_name}"
        #MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.bigbigmdm.imsprog"
        MACOSX_BUNDLE_ICON_FILE "${target_name}.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.2"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.2-10"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2023-2026 Mikhail Medvedev"
    )
    
    # App icons
    # Ensure the generated .icns file is placed into the bundle's Resources directory
    set_source_files_properties("${TARGET_ICNS}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        
    # Link the icon file to the target so CMake tracks and triggers the custom command
    target_sources(${target_name} PRIVATE "${TARGET_ICNS}")
endfunction()

add_subdirectory("IMSProg_programmer")
add_subdirectory("IMSProg_editor")
add_subdirectory("IMSProg_database_update")
