#-------------------------------------------------
#
# Project created by QtCreator 2023-05-26T10:45:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IMSProg_editor
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
        ezp_chip_editor.cpp \
        delegates.cpp

HEADERS += \
        ezp_chip_editor.h \
        delegates.h

FORMS += \
        ezp_chip_editor.ui

TRANSLATIONS += language/chipEditor_ru_RU.ts \
        language/chipEditor_es_ES.ts \
        language/chipEditor_hu_HU.ts \
        language/chipEditor_de_DE.ts \
        language/chipEditor_zh_CN.ts \
        language/chipEditor_zh_TW.ts \
        language/chipEditor_uk_UA.ts \
        language/chipEditor_pt_BR.ts \
        language/chipEditor_it_IT.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
