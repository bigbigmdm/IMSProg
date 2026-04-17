#-------------------------------------------------
#
# Project created by QtCreator 2026-04-10T12:21:41
#
#-------------------------------------------------

QT       += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IMSProg_database_update
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
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui
TRANSLATIONS += language/chipUpdater_ru_RU.ts \
         language/chipUpdater_es_ES.ts \
         language/chipUpdater_de_DE.ts \
         language/chipUpdater_zh_CN.ts \
         language/chipUpdater_zh_TW.ts \
         language/chipUpdater_uk_UA.ts \
         language/chipUpdater_hu_HU.ts \
         language/chipUpdater_pt_BR.ts \
         language/chipUpdater_it_IT.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
