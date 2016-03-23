TARGET = ubuntutheme

PLUGIN_TYPE = platformthemes
PLUGIN_CLASS_NAME = UbuntuThemePlugin
load(qt_plugin)

QT += core-private gui-private platformsupport-private
CONFIG += no_keywords link_pkgconfig
PKGCONFIG += gio-2.0
LIBS += ../common/libqpa-ubuntucommon.a

DESTDIR = ./

HEADERS += \
    ubuntuthemeplugin.h

SOURCES += \
    ubuntuthemeplugin.cpp

OTHER_FILES += \
    ubuntu.json
