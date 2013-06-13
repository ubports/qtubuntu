TARGET = qubuntucommon
TEMPLATE = lib

QT += core-private gui-private platformsupport-private sensors-private

DEFINES += MESA_EGL_NO_X11_HEADERS
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = integration.cc \
          window.cc \
          screen.cc \
          input.cc \
          clipboard.cc

HEADERS = integration.h \
          window.h \
          screen.h \
          input.h \
          clipboard.h

CONFIG += static plugin link_prl

PRE_TARGETDEPS = ../base/libubuntubase.a

INCLUDEPATH += ..
LIBS += -L../base -Wl,--whole-archive -lubuntubase -Wl,--no-whole-archive
