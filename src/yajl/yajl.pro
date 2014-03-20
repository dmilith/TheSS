# ServeD natives - JSON CPP
#

include(../Common.pro)

TARGET = ../yajl
TEMPLATE = lib
# DEFINES += YAJL_LEXER_DEBUG
CONFIG += staticlib

SOURCES += *.c
HEADERS += *.h api/*.h
