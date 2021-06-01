TARGET = qpa-ubuntumirclient
TEMPLATE = lib

QT -= gui
QT += core-private egl_support-private fontdatabase_support-private eventdispatcher_support-private linuxaccessibility_support-private theme_support-private dbus

CONFIG += plugin no_keywords qpa/genericunixfontdatabase

DEFINES += MESA_EGL_NO_X11_HEADERS
# CONFIG += c++11 # only enables C++0x
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -Werror -Wall
QMAKE_LFLAGS += -std=c++11 -Wl,-no-undefined

CONFIG += link_pkgconfig
PKGCONFIG += egl mirclient url-dispatcher-1 xkbcommon libcontent-hub

SOURCES = \
    qmirclientbackingstore.cpp \
    qmirclientclipboard.cpp \
    qmirclientcursor.cpp \
    qmirclientdebugextension.cpp \
    qmirclientdesktopwindow.cpp \
    qmirclientglcontext.cpp \
    qmirclientinput.cpp \
    qmirclientintegration.cpp \
    qmirclientnativeinterface.cpp \
    qmirclientplatformservices.cpp \
    qmirclientplugin.cpp \
    qmirclientscreen.cpp \
    qmirclientscreenobserver.cpp \
    qmirclientwindow.cpp \
    qmirclientappstatecontroller.cpp

HEADERS = \
    qmirclientbackingstore.h \
    qmirclientclipboard.h \
    qmirclientcursor.h \
    qmirclientdebugextension.h \
    qmirclientdesktopwindow.h \
    qmirclientglcontext.h \
    qmirclientinput.h \
    qmirclientintegration.h \
    qmirclientnativeinterface.h \
    qmirclientorientationchangeevent_p.h \
    qmirclientplatformservices.h \
    qmirclientplugin.h \
    qmirclientscreenobserver.h \
    qmirclientscreen.h \
    qmirclientwindow.h \
    qmirclientlogging.h \
    qmirclientappstatecontroller.h \
    ../shared/ubuntutheme.h

OTHER_FILES += \
    ubuntumirclient.json

# Installation path
target.path +=  $$[QT_INSTALL_PLUGINS]/platforms

INSTALLS += target
