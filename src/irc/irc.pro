# ServeD natives - low level notification mechanism - IRC module
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

TARGET = ../irc
TEMPLATE = lib
CONFIG += staticlib

HEADERS   += irc.h

SOURCES   += irc.cc
