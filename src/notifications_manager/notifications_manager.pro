# ServeD natives - Notifications manager
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
TARGET = ../../bin/svdnom
TEMPLATE = app

HEADERS   += ../notifications/notifications.h manager.h

SOURCES   += ../service_spawner/utils.cc manager.cc

mac {
      LIBS      += ../libnotifications.a ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncurses
} else {
      LIBS      += ../libnotifications.a ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncursesw
}