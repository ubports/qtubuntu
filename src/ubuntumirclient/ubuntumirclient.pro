TARGET = qpa-ubuntumirclient
TEMPLATE = lib

QT -= gui
QT += core-private platformsupport-private dbus

CONFIG += plugin no_keywords qpa/genericunixfontdatabase

DEFINES += MESA_EGL_NO_X11_HEADERS
# CONFIG += c++11 # only enables C++0x
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -Werror -Wall
QMAKE_LFLAGS += -std=c++11 -Wl,-no-undefined

CONFIG += link_pkgconfig
PKGCONFIG += egl mirclient ubuntu-platform-api xkbcommon libcontent-hub

SOURCES = \
    backingstore.cpp \
    clipboard.cpp \
    cursor.cpp \
    debugextension.cpp \
    desktopwindow.cpp \
    glcontext.cpp \
    input.cpp \
    integration.cpp \
    nativeinterface.cpp \
    platformservices.cpp \
    plugin.cpp \
    screen.cpp \
    screenobserver.cpp \
    window.cpp \
    appstatecontroller.cpp

HEADERS = \
    backingstore.h \
    clipboard.h \
    cursor.h \
    debugextension.h \
    desktopwindow.h \
    glcontext.h \
    input.h \
    integration.h \
    nativeinterface.h \
    orientationchangeevent_p.h \
    platformservices.h \
    plugin.h \
    screenobserver.h \
    screen.h \
    window.h \
    logging.h \
    appstatecontroller.h

OTHER_FILES += \
    ubuntumirclient.json

# Installation path
target.path +=  $$[QT_INSTALL_PLUGINS]/platforms

INSTALLS += target
