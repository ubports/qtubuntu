TARGET = ubuntu

PLUGIN_TYPE = platformthemes
PLUGIN_CLASS_NAME = UbuntuPlugin
load(qt_plugin)

QT += core-private gui-private platformsupport-private dbus
CONFIG += no_keywords link_pkgconfig
PKGCONFIG += gio-2.0

DBUS_INTERFACES += com.ubuntu.MenuRegistrar.xml

QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -Werror -Wall
QMAKE_LFLAGS += -std=c++11 -Wl,-no-undefined

DESTDIR = ./

HEADERS += \
    ubuntuthemeplugin.h \
    theme.h \
    gmenumodelexporter.h \
    gmenumodelplatformmenu.h \
    logging.h \
    menuregistrar.h \
    registry.h

SOURCES += \
    ubuntuthemeplugin.cpp \
    theme.cpp \
    gmenumodelexporter.cpp \
    gmenumodelplatformmenu.cpp \
    menuregistrar.cpp \
    registry.cpp

OTHER_FILES +=

DISTFILES += \
    ubuntu.json
