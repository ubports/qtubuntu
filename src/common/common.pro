TARGET = qpa-ubuntucommon
TEMPLATE = lib
CONFIG += staticlib

QT -= gui
QT += core-private platformsupport-private dbus

CONFIG += plugin no_keywords qpa/genericunixfontdatabase
DBUS_INTERFACES += com.ubuntu.MenuSurfaceRegistrar.xml

QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -Werror -Wall
QMAKE_LFLAGS += -std=c++11 -Wl,-no-undefined

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0
INCLUDEPATH += $$top_builddir/src/common

HEADERS = theme.h \
          gmenumodelexporter.h \
          gmenumodelplatformmenu.h \
          menuregistrar.h \
          menusurfaceregistrar_interface.h \
          registry.h

SOURCES = theme.cpp \
          gmenumodelexporter.cpp \
          gmenumodelplatformmenu.cpp \
          menuregistrar.cpp \
          menusurfaceregistrar_interface.cpp \
          registry.cpp
