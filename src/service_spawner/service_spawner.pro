# ServeD natives - TheSS - (c) 2o13-2o15 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network websockets
HEADERS   += \
             process.h \
             service.h \
             ../notifications/notifications.h \
             service_config.h \
             service_watcher.h \
             user_watcher.h \
             file_events_manager.h \
             api.h
SOURCES   += \
             process.cc \
             service.cc \
             service_config.cc \
             service_watcher.cc \
             user_watcher.cc \
             api.cc \
             file_events_manager.cc \
             ../notifications/notifications.cc \
             service_spawner.cc
LIBS      += ../liblogger.a ../libcore.a ../libyajl.a ../libhiredis.a ../libquazip.a -lz
TARGET    = ../../bin/svdss

