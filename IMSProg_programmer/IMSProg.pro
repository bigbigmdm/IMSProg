#-------------------------------------------------
#
# Project created by QtCreator 2023-07-14T12:38:17
#
#-------------------------------------------------

QT       += core gui
LIBS += -lusb-1.0
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IMSProg
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
       qhexedit.cpp \
       chunks.cpp \
       commands.cpp \
       dialogsp.cpp \
       dialogrp.cpp \
       searchdialog.cpp \
       bitbang_microwire.c \
       ch341a_gpio.c \
       ch341a_i2c.c \
       ch341a_spi.c \
       flashcmd_api.c \
       i2c_eeprom.c \
       mw_eeprom.c \
       spi_controller.c \
       spi_eeprom.c \
       spi_nand_flash.c \
       spi_nor_flash.c \
       timer.c \
    dialogabout.cpp

HEADERS += \
        mainwindow.h \
       qhexedit.h \
       chunks.h \
       commands.h \
       dialogsp.h \
       dialogrp.h \
       searchdialog.h \
       bitbang_microwire.h \
       ch341a_gpio.h \
       ch341a_i2c.h \
       ch341a_spi.h \
       flashcmd_api.h \
       i2c_eeprom_api.h \
       mw_eeprom_api.h \
       nandcmd_api.h \
       res.h \
       snorcmd_api.h \
       spi_controller.h \
       spi_eeprom.h \
       spi_eeprom_api.h \
       spi_nand_flash.h \
       timer.h \
       types.h \
    dialogabout.h


FORMS += \
        mainwindow.ui \
        dialogsp.ui \
        dialogrp.ui \
        searchdialog.ui \
    dialogabout.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    recource.qrc

DISTFILES +=
