# ServeD natives - TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += service.h \
             ../notifications/notifications.h \
             ../death_watch/death_watch.h \
             process.h \
             service_config.h \
             utils.h \
             service_watcher.h \
             user_watcher.h \
             file_events_manager.h \
             cron_entry.h \
             logger.h
SOURCES   += service.cc \
             process.cc \
             service_config.cc \
             utils.cc \
             service_watcher.cc \
             user_watcher.cc \
             file_events_manager.cc \
             ../death_watch/death_watch.cc \
             ../notifications/notifications.cc \
             service_spawner.cc \
             cron_entry.cc \
             logger.cc
LIBS      += -lz ../liblogger.a ../libjsoncpp.a ../libhiredis.a ../libquazip.a
TARGET    = ../../bin/svdss
