# ServeD natives - hiredis module
#

include(../Common.pro)

TARGET = ../hiredis
TEMPLATE = lib
CONFIG += staticlib

# QMAKE_CXXFLAGS += -w

SOURCES += *.c

HEADERS += *.h adapters/*.h
