# ServeD natives - JSON CPP
#

include(../Common.pro)

TARGET = ../yajl
TEMPLATE = lib
CONFIG += staticlib

SOURCES += *.c
HEADERS += *.h api/*.h
