# ServeD natives - TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../core/json_api.h \
             service.h \
             ../notifications/notifications.h \
             ../death_watch/death_watch.h \
             process.h \
             service_config.h \
             ../core/utils.h \
             service_watcher.h \
             user_watcher.h \
             file_events_manager.h \
             cron_entry.h \
             ../core/logger.h
SOURCES   += ../core/json_api.cc \
             service.cc \
             process.cc \
             service_config.cc \
             ../core/utils.cc \
             service_watcher.cc \
             user_watcher.cc \
             file_events_manager.cc \
             ../death_watch/death_watch.cc \
             ../notifications/notifications.cc \
             service_spawner.cc \
             cron_entry.cc \
             ../core/logger.cc
LIBS      += -lz ../liblogger.a ../libyajl.a ../libhiredis.a ../libquazip.a
TARGET    = ../../bin/svdss
