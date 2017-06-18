QT += widgets

LIBS += -luuid
LIBS += -lrt

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
