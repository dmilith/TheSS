# ServeD natives - low level notification mechanism
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
TARGET = ../notifications
TEMPLATE = lib
CONFIG += staticlib

HEADERS   += notifications.h

SOURCES   += notifications.cc
