# ServeD natives - JSON CPP
#

include(../Common.pro)

TARGET = ../jsoncpp
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -w

SOURCES += *.cpp

HEADERS += json/*.h
