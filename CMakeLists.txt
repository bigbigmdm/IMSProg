cmake_minimum_required(VERSION 3.10.0)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 9)
set(VERSION_PATCH 1)

project(imsprog 
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    LANGUAGES C CXX
)

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
    message(STATUS "Configuring macOS Application Bundle ${target_name}.app v${imsprog_VERSION} on Darwin")

    # Define paths for the source image and icns file
    message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/${target_icon}")
    set(TARGET_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/${target_icon}")

    # Set target properties for the macOS application bundle
    set_target_properties(${target_name} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${target_name}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.bigbigmdm.imsprog.${PROJECT_NAME}"
        MACOSX_BUNDLE_ICON_FILE "${target_name}.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${imsprog_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${imsprog_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2023-2026 Mikhail Medvedev"
    )
    
    # App icons
    # Ensure the .icns file is placed into the bundle's Resources directory
    set_source_files_properties("${TARGET_ICNS}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        
    # Link the icon file to the target so CMake tracks and triggers the custom command
    target_sources(${target_name} PRIVATE "${TARGET_ICNS}")
endfunction()

add_subdirectory("IMSProg_programmer")
add_subdirectory("IMSProg_editor")
add_subdirectory("IMSProg_database_update")
