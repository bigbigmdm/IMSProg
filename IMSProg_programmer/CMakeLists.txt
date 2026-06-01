cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg LANGUAGES C CXX)

include(FindPkgConfig)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_INSTALL_DO_STRIP FALSE)
endif()

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Set the CMAKE_INSTALL_PREFIX to /usr if not specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "The default install prefix on macOS" FORCE)

      # Set up Qt paths on macOS
      # If user hasn't set CMAKE_PREFIX_PATH, try to find Qt automatically
      if(NOT CMAKE_PREFIX_PATH)
          # Check for Qt6 via Homebrew
          if(EXISTS "/usr/local/opt/qt@6")
              set(CMAKE_PREFIX_PATH "/usr/local/opt/qt@6" CACHE PATH "Path to Qt6 on macOS" FORCE)
              message(STATUS "Found Qt6 at /usr/local/opt/qt@6")
          # Check for Qt5 via Homebrew
          elseif(EXISTS "/usr/local/opt/qt@5")
              set(CMAKE_PREFIX_PATH "/usr/local/opt/qt@5" CACHE PATH "Path to Qt5 on macOS" FORCE)
              message(STATUS "Found Qt5 at /usr/local/opt/qt@5")
          # Standard Qt path from Homebrew (without version)
          elseif(EXISTS "/usr/local/opt/qt")
              set(CMAKE_PREFIX_PATH "/usr/local/opt/qt" CACHE PATH "Path to Qt on macOS" FORCE)
              message(STATUS "Found Qt at /usr/local/opt/qt")
          endif()
      endif()
  else()
      set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "The default install prefix" FORCE)
  endif()
endif()

# Set the CMAKE_INSTALL_BINDIR to /bin if not specified
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR "bin" CACHE PATH "user executables (bin)")
endif()

# Set the CMAKE_INSTALL_DATAROOTDIR to /share if not specified
if(NOT DEFINED CMAKE_INSTALL_DATAROOTDIR)
  set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE PATH "read-only architecture-independent data root (share)")
endif()

# Set the CMAKE_INSTALL_LIBDIR to /lib if not specified
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "/lib" CACHE PATH "read-only single-machine data (lib)")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Option to force using Qt5
option(FORCE_QT5 "Force using Qt5 instead of Qt6" OFF)

if(FORCE_QT5)
    message(STATUS "Forced Qt5 mode for IMSProg")

    # On macOS, additionally check the path to Qt5
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT CMAKE_PREFIX_PATH)
        if(EXISTS "/usr/local/opt/qt@5")
            set(CMAKE_PREFIX_PATH "/usr/local/opt/qt@5" CACHE PATH "Path to Qt5 on macOS" FORCE)
            message(STATUS "Setting CMAKE_PREFIX_PATH to /usr/local/opt/qt@5 for Qt5")
        elseif(EXISTS "/usr/local/opt/qt")
            set(CMAKE_PREFIX_PATH "/usr/local/opt/qt" CACHE PATH "Path to Qt on macOS" FORCE)
            message(STATUS "Setting CMAKE_PREFIX_PATH to /usr/local/opt/qt for Qt5")
        endif()
    endif()

    find_package(Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools)
    set(QT_VERSION_MAJOR 5)
else()
    # First try Qt6
    find_package(Qt6 QUIET COMPONENTS Core Widgets)
    if(TARGET Qt6::Core AND TARGET Qt6::Widgets)
        message(STATUS "Using Qt6 for IMSProg")
        set(QT_VERSION_MAJOR 6)
        find_package(Qt6LinguistTools REQUIRED)
    else()
        message(STATUS "Qt6 not found, using Qt5 for IMSProg")

        # On macOS, additionally check the path to Qt5
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT CMAKE_PREFIX_PATH)
            if(EXISTS "/usr/local/opt/qt@5")
                set(CMAKE_PREFIX_PATH "/usr/local/opt/qt@5" CACHE PATH "Path to Qt5 on macOS" FORCE)
                message(STATUS "Setting CMAKE_PREFIX_PATH to /usr/local/opt/qt@5 for Qt5")
            elseif(EXISTS "/usr/local/opt/qt")
                set(CMAKE_PREFIX_PATH "/usr/local/opt/qt" CACHE PATH "Path to Qt on macOS" FORCE)
                message(STATUS "Setting CMAKE_PREFIX_PATH to /usr/local/opt/qt for Qt5")
            endif()
        endif()

        find_package(Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools)
        set(QT_VERSION_MAJOR 5)
    endif()
endif()

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(UDEVDIR "/usr/lib/udev")
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr" OR CMAKE_INSTALL_PREFIX STREQUAL "/" AND NOT CMAKE_CROSSCOMPILING)
# /usr and / install prefixes at treated by cmake GNUInstallDirs as
# synonym for "system location". In this case we can look up the correct udevdir
# using pkg-config.
# See: https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html#special-cases
find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(PKGCONFIG_UDEV udev)
    if (PKGCONFIG_UDEV_FOUND)
      execute_process(
        COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=udevdir udev
          OUTPUT_VARIABLE PKGCONFIG_UDEVDIR
          OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if(PKGCONFIG_UDEVDIR)
        file(TO_CMAKE_PATH "${PKGCONFIG_UDEVDIR}" UDEVDIR)
      endif()
    endif()
  endif()
endif()
endif()

# Translation files
set(TS_FILES
    language/chipProgrammer_ru_RU.ts
    language/chipProgrammer_de_DE.ts
    language/chipProgrammer_es_ES.ts
    language/chipProgrammer_zh_CN.ts
    language/chipProgrammer_zh_TW.ts
    language/chipProgrammer_uk_UA.ts
    language/chipProgrammer_hu_HU.ts
    language/chipProgrammer_pt_BR.ts
    language/chipProgrammer_it_IT.ts
)

if (QT_VERSION_MAJOR EQUAL 5)
    qt5_add_translation(QM_FILES ${TS_FILES})
else()
    qt_add_translation(QM_FILES ${TS_FILES})
endif()

add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
main.cpp
mainwindow.cpp
qhexedit.cpp
chunks.cpp
commands.cpp
dialogbbm.cpp
dialogsp.cpp
dialogrp.cpp
dialogsfdp.cpp
dialognandsr.cpp
dialogsecurity.cpp
dialognandsecurity.cpp
dialogcompare.cpp
dialogsr.cpp
dialogsetaddr.cpp
dialogfill.cpp
hexutility.cpp
searchdialog.cpp
dialoginfo.cpp
ch34x_i2c.c
ch347.c
color_manager.cpp
dialogabout.cpp
mainwindow.h
qhexedit.h
chunks.h
commands.h
color_manager.h
dialogbbm.h
dialogsp.h
dialogrp.h
dialogsfdp.h
dialogsecurity.h
dialognandsecurity.h
dialognandsr.h
dialogcompare.h
dialogsr.h
dialoginfo.h
dialogsetaddr.h
dialogfill.h
hexutility.h
searchdialog.h
ch34x_i2c.h
ch347.h
dialogabout.h
mainwindow.ui
dialogbbm.ui
dialogsp.ui
dialogrp.ui
dialogsfdp.ui
dialognandsr.ui
dialogsecurity.ui
dialognandsecurity.ui
dialogcompare.ui
dialogsr.ui
dialogsetaddr.ui
dialoginfo.ui
dialogfill.ui
searchdialog.ui
dialogabout.ui
recource.qrc
SNANDer/spi_controller.h
SNANDer/ch341a_spi.h
SNANDer/nandcmd_api.h
SNANDer/types.h
SNANDer/timer.h
SNANDer/snorcmd_api.h
SNANDer/flashcmd_api.h
SNANDer/bitbang_microwire.h
SNANDer/spi_nor_flash.c
SNANDer/mw_eeprom.c
SNANDer/ch341a_gpio.c
SNANDer/spi_eeprom_api.h
SNANDer/spi_controller.c
SNANDer/res.h
SNANDer/ch341a_spi.c
SNANDer/timer.c
SNANDer/mw_eeprom_api.h
SNANDer/flashcmd_api.c
SNANDer/ch341a_gpio.h
SNANDer/bitbang_microwire.c
${QM_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/SNANDer")
target_include_directories(${PROJECT_NAME} PRIVATE "${LIBUSB_INCLUDE_DIRS}")

if (QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets ${LIBUSB_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} Qt::Core Qt::Widgets ${LIBUSB_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/IMSProg64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/database/IMSProg.Dat" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/71-CH341.rules" DESTINATION "${UDEVDIR}/rules.d")
endif()

install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/index.html" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/imsprog/html")
install(DIRECTORY "${PROJECT_ROOT}/img/"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/imsprog/html")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
