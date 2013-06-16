# ServeD natives - Notifications manager
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
TARGET = ../../bin/svdnom
TEMPLATE = app

HEADERS   += ../notifications/notifications.h manager.h ../service_spawner/process.h

SOURCES   += ../service_spawner/utils.cc ../service_spawner/process.cc manager.cc

mac {
      LIBS      += ../libnotifications.a ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncurses
} else {
      LIBS      += ../libnotifications.a ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncursesw
}