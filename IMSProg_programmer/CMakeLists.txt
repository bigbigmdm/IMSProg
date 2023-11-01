cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(IMSProg LANGUAGES C CXX)

# Set the CMAKE_INSTALL_PREFIX to /usr if not specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "The default install prefix" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt5Widgets REQUIRED)
find_package(LibUSB REQUIRED)

add_executable(${PROJECT_NAME}

main.cpp
mainwindow.cpp
qhexedit.cpp
chunks.cpp
commands.cpp
dialogsp.cpp
dialogrp.cpp
dialogsfdp.cpp
searchdialog.cpp
dialoginfo.cpp
bitbang_microwire.c
ch341a_gpio.c
ch341a_i2c.c
ch341a_spi.c
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
dialogsp.h
dialogrp.h
dialogsfdp.h
dialoginfo.h
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
dialoginfo.ui
searchdialog.ui
dialogabout.ui
recource.qrc
	)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${LibUSB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/IMSProg)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/IMSProg.desktop" DESTINATION /usr/share/applications)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/IMSProg64.png" DESTINATION /usr/share/pixmaps)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/database/IMSProg.Dat" DESTINATION /opt/IMSProg)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/other/99-CH341.rules" DESTINATION /etc/udev/rules.d)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipProgrammer_ru_RU.qm" DESTINATION /opt/IMSProg/language)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipProgrammer_es_ES.qm" DESTINATION /opt/IMSProg/language)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipProgrammer_de_DE.qm" DESTINATION /opt/IMSProg/language)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipProgrammer_zn_CH.qm" DESTINATION /opt/IMSProg/language)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/language/chipProgrammer_uk_UA.qm" DESTINATION /opt/IMSProg/language)
