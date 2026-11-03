cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg_database_update LANGUAGES CXX)

# --- Force /usr prefix on Linux by default ---
if(NOT APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Default install prefix for Linux" FORCE)
    endif()
endif()

# Standard cross-platform installation paths (bin, share, etc.)
include(GNUInstallDirs)

# --- Automatic path detection on macOS (Homebrew / MacPorts) ---
if(APPLE)
    if(EXISTS "/opt/local")
        # MacPorts
        list(APPEND CMAKE_PREFIX_PATH "/opt/local")
        list(APPEND PKG_CONFIG_PATH "/opt/local/lib/pkgconfig")
        message(STATUS "Configured for MacPorts at /opt/local")
    elseif(EXISTS "/opt/homebrew")
        # Homebrew Apple Silicon (M1/M2/M3/M4)
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
        list(APPEND PKG_CONFIG_PATH "/opt/homebrew/lib/pkgconfig")
        message(STATUS "Configured for Homebrew Apple Silicon at /opt/homebrew")
    elseif(EXISTS "/usr/local/opt")
        # Homebrew Intel
        list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt")
        message(STATUS "Configured for Homebrew Intel at /usr/local")
    endif()
endif()

# --- Find Qt (Qt6 with automatic fallback to Qt5) ---
find_package(Qt6 QUIET COMPONENTS Core Widgets Network)
if(TARGET Qt6::Core AND TARGET Qt6::Widgets AND TARGET Qt6::Network)
    message(STATUS "Using Qt6 for IMSProg_database_update")
    set(QT_VERSION_MAJOR 6)
    find_package(Qt6LinguistTools REQUIRED)
else()
    message(STATUS "Qt6 not found, falling back to Qt5 for IMSProg_database_update")
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)
    set(QT_VERSION_MAJOR 5)
endif()

# --- Translation files ---
set(TS_FILES
    language/chipUpdater_ru_RU.ts
    language/chipUpdater_es_ES.ts
    language/chipUpdater_de_DE.ts
    language/chipUpdater_zh_CN.ts
    language/chipUpdater_zh_TW.ts
    language/chipUpdater_uk_UA.ts
    language/chipUpdater_hu_HU.ts
    language/chipUpdater_pt_BR.ts
    language/chipUpdater_it_IT.ts
)

if (QT_VERSION_MAJOR EQUAL 5)
    qt5_add_translation(QM_FILES ${TS_FILES})
else()
    qt_add_translation(QM_FILES ${TS_FILES})
endif()

add_custom_target(translations2 DEPENDS ${QM_FILES})

# --- macOS Icon asset definition ---
set(MACOS_ICON_FILE "img/icons/apple/IMSProg_database_update.icns")

if(APPLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MACOS_ICON_FILE}")
    set_source_files_properties("${MACOS_ICON_FILE}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

# --- Executable target ---
add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resource.qrc
    ${QM_FILES}
    $<$<BOOL:${APPLE}>:${MACOS_ICON_FILE}>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets Qt::Network)
endif()

# --- macOS Bundle configuration ---
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.bigbigmdm.imsprog.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.9.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.9.1"
        MACOSX_BUNDLE_ICON_FILE "IMSProg_database_update.icns"
    )

    set(BUNDLE_DEST "${CMAKE_INSTALL_PREFIX}/Applications")
else()
    set(BUNDLE_DEST "${CMAKE_INSTALL_BINDIR}")
endif()

# --- Installation rules ---

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION ${BUNDLE_DEST}           # Bundles into .app on macOS
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # Standard binary for Linux/Flatpak
)

if(APPLE)
    # Automatically bundle .qm files inside .app/Contents/Resources for local installation
    install(FILES ${QM_FILES} DESTINATION "${PROJECT_NAME}.app/Contents/Resources")
else()
    # --- Specific rules for Linux / Flatpak ---
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog_database_update.desktop" 
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    
    # Application icons
    foreach(SIZE 16x16 22x22 24x24 32x32 48x48 64x64 128x128 256x256 512x512 scalable)
        install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/img/icons/${SIZE}/"
                DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${SIZE}/apps"
                FILES_MATCHING PATTERN "*.png" PATTERN "*.svg")
    endforeach()

    # Translations and Linux system files
    install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_database_update.1.gz" 
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog_database_update.metainfo.xml" 
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
endif()
