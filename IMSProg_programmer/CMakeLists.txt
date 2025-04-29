cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg LANGUAGES C CXX)

# Set the CMAKE_INSTALL_PREFIX to /usr if not specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "The default install prefix on macOS" FORCE)
  # Set Qt5 path for macOS
  	set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}/opt/qt@5" CACHE PATH "Path to Qt5 on macOS" FORCE)
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

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt5Widgets REQUIRED)
find_package(LibUSB REQUIRED)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(UDEVDIR "/usr/lib/udev")
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr" OR CMAKE_INSTALL_PREFIX STREQUAL "/" AND NOT CMAKE_CROSSCOMPILING)
# /usr and / install prefixes at treated by cmake GNUInstallDirs as
# synonym for "system location". In this case we can look up the correct udevdir
# using pkg-config.
# See: https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html#special-cases
find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules( PKGCONFIG_UDEV udev)
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
# Qt5LinguistTools
find_package(Qt5 REQUIRED COMPONENTS LinguistTools)

# Translation files
set(TS_FILES
    language/chipProgrammer_ru_RU.ts
    language/chipProgrammer_de_DE.ts
    language/chipProgrammer_es_ES.ts
    language/chipProgrammer_zh_CN.ts
    language/chipProgrammer_uk_UA.ts
    language/chipProgrammer_hu_HU.ts
    language/chipProgrammer_pt_BR.ts
    language/chipProgrammer_it_IT.ts
)

qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}

main.cpp
mainwindow.cpp
qhexedit.cpp
chunks.cpp
commands.cpp
dialogsp.cpp
dialogrp.cpp
dialogsfdp.cpp
dialogsecurity.cpp
dialogsr.cpp
dialogsetaddr.cpp
hexutility.cpp
searchdialog.cpp
dialoginfo.cpp
bitbang_microwire.c
ch341a_gpio.c
ch341a_i2c.c
ch341a_spi.c
color_manager.cpp
flashcmd_api.c
i2c_eeprom.c
mw_eeprom.c
spi_controller.c
spi_eeprom.c
spi_nand_flash.c
spi_nor_flash.c
timer.c
dialogabout.cpp
mainwindow.h
qhexedit.h
chunks.h
commands.h
color_manager.h
dialogsp.h
dialogrp.h
dialogsfdp.h
dialogsecurity.h
dialogsr.h
dialoginfo.h
dialogsetaddr.h
hexutility.h
searchdialog.h
bitbang_microwire.h
ch341a_gpio.h
ch341a_i2c.h
ch341a_spi.h
flashcmd_api.h
i2c_eeprom_api.h
mw_eeprom_api.h
nandcmd_api.h
res.h
snorcmd_api.h
spi_controller.h
spi_eeprom.h
spi_eeprom_api.h
spi_nand_flash.h
timer.h
types.h
dialogabout.h
mainwindow.ui
dialogsp.ui
dialogrp.ui
dialogsfdp.ui
dialogsecurity.ui
dialogsr.ui
dialogsetaddr.ui
dialoginfo.ui
searchdialog.ui
dialogabout.ui
recource.qrc
${QM_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${LibUSB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/IMSProg64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/database/IMSProg.Dat" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/71-CH341.rules" DESTINATION "${UDEVDIR}/rules.d")
endif()

install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/imsprog")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_database_update"
PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_database_update.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/IMSProg_database_update.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg_database_update.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/index.html" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/imsprog/html")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/io.github.bigbigmdm.imsprog_database_update.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
