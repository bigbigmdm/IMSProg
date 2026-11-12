cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg_database_update LANGUAGES CXX)

# --- Force /usr prefix on Linux by default ---
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Default install prefix for Linux" FORCE)
    endif()
endif()

# Standard cross-platform installation paths (bin, share, etc.)
include(GNUInstallDirs)

# --- Automatic path detection on macOS (Homebrew / MacPorts) ---
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
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

# Set the CMAKE_INSTALL_BINDIR to /bin if not specified
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE PATH "user executables (bin)")
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

# --- Executable target ---
add_executable(${PROJECT_NAME} MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resource.qrc
    ${QM_FILES}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    configure_macos_bundle(${PROJECT_NAME} "img/icons/apple/${PROJECT_NAME}.icns")    
    if(BUILD_ON_MACPORTS)
        set(BUNDLE_DEST "/Applications/MacPorts")
    else()
        set(BUNDLE_DEST "/Applications")
    endif()    
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets Qt::Network)
endif()

# --- Installation rules ---
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION ${BUNDLE_DEST}               # Bundles into .app on macOS
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}     # Standard binary for Linux/Flatpak
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Automatically bundle .qm files inside .app/Contents/Resources for local installation
    install(FILES ${QM_FILES} DESTINATION "${PROJECT_NAME}.app/Contents/Resources")
else()
    # --- Specific rules for Linux / Flatpak ---
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog.database_update.desktop" 
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
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog.database_update.metainfo.xml" 
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
endif()
