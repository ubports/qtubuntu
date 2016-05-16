TARGET = ubuntu

PLUGIN_TYPE = platformthemes
PLUGIN_CLASS_NAME = UbuntuPlugin
load(qt_plugin)

QT += core-private gui-private platformsupport-private
CONFIG += no_keywords link_pkgconfig
PKGCONFIG += gio-2.0
LIBS += ../common/libqpa-ubuntucommon.a
INCLUDEPATH += ../common

DESTDIR = ./

HEADERS += \
    ubuntuthemeplugin.h

SOURCES += \
    ubuntuthemeplugin.cpp

OTHER_FILES += \
    ubuntu.json
