# ServeD natives - low level notification mechanism
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network

HEADERS   += notifications.h ../service_spawner/process.h
SOURCES   += notifier.cc notifications.cc ../service_spawner/utils.cc ../service_spawner/process.cc

LIBS += -lz ../liblogger.a ../libjsoncpp.a ../libhiredis.a ../libquazip.a

TARGET = ../../bin/svdnotify
