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

HEADERS   += notifications.h ../service_spawner/process.h

SOURCES   += notifications.cc ../service_spawner/utils.cc ../service_spawner/process.cc

mac {
      LIBS      += ../libnotifications.a ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncurses
} else {
      LIBS      += ../libnotifications.a ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncursesw
}
