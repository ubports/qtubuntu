TARGET = ubuntuappmenu
TEMPLATE = lib

QT -= gui
QT += core-private platformsupport-private dbus

CONFIG += plugin no_keywords

# CONFIG += c++11 # only enables C++0x
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -Werror -Wall
QMAKE_LFLAGS += -std=c++11 -Wl,-no-undefined

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0

DBUS_INTERFACES += com.ubuntu.MenuRegistrar.xml

HEADERS += \
    theme.h \
    gmenumodelexporter.h \
    gmenumodelplatformmenu.h \
    logging.h \
    menuregistrar.h \
    registry.h \
    themeplugin.h \
    qtubuntuextraactionhandler.h

SOURCES += \
    theme.cpp \
    gmenumodelexporter.cpp \
    gmenumodelplatformmenu.cpp \
    menuregistrar.cpp \
    registry.cpp \
    themeplugin.cpp \
    qtubuntuextraactionhandler.cpp

OTHER_FILES += \
    ubuntuappmenu.json

# Installation path
target.path +=  $$[QT_INSTALL_PLUGINS]/platformthemes

INSTALLS += target
