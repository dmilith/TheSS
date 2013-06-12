# ServeD natives - Notifications manager
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
TARGET = ../../svdnom
TEMPLATE = app

HEADERS   += ../notifications/notifications.h manager.h

SOURCES   += ../notifications/notifications.cc ../service_spawner/utils.cc manager.cc

mac {
      LIBS      += ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncurses
} else {
      LIBS      += ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncursesw
}