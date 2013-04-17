# ServeD natives - Cute Logger
#

include(../Common.pro)

TARGET = ../logger
TEMPLATE = lib
CONFIG += staticlib

DEFINES += CUTELOGGER_LIBRARY


SOURCES += Logger.cpp \
           AbstractAppender.cpp \
           AbstractStringAppender.cpp \
           ConsoleAppender.cpp \
           FileAppender.cpp

HEADERS += Logger.h \
           CuteLogger_global.h \
           AbstractAppender.h \
           AbstractStringAppender.h \
           ConsoleAppender.h \
           FileAppender.h
