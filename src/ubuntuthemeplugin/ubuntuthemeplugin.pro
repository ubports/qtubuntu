TARGET = ubuntu

PLUGIN_TYPE = platformthemes
PLUGIN_CLASS_NAME = UbuntuThemePlugin
load(qt_plugin)

QT += core-private gui-private platformsupport-private
CONFIG += no_keywords link_pkgconfig
PKGCONFIG += gio-2.0

DESTDIR = ./

HEADERS += \
    theme.h \
    ubuntuthemeplugin.h \
    gmenumodelplatformmenu.h

SOURCES += \
    theme.cpp \
    ubuntuthemeplugin.cpp \
    gmenumodelplatformmenu.cpp

OTHER_FILES += \
    ubuntu.json
