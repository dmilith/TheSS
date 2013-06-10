# ServeD natives - low level notification center
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
