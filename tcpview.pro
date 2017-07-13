QT += widgets

LIBS += -luuid

HEADERS = window.h \
        tableheadercaption.h \
    updatethread.h \
    datasource.h \
    rootmodule.h \
    buffer.h

SOURCES = main.cpp \
        window.cpp \
    updatethread.cpp \
    datasource.cpp \
    rootmodule.cpp \
    buffer.cpp

CONFIG += c++11

CONFIG(debug, release|debug):DEFINES += _DEBUG

RESOURCES   = tcpview.qrc

DISTFILES += \
    LICENSE \
    README.md \
    data/tcpview.desktop \
    debian/source/format \
    debian/source/format \
    debian/compat \
    debian/control \
    debian/copyright \
    debian/changelog \
    debian/rules \
    debian/tcpview.install

