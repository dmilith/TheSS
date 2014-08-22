# ServeD natives - TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += \
             logger.h \
             process.h \
             service.h \
             ../notifications/notifications.h \
             ../death_watch/death_watch.h \
             service_config.h \
             service_watcher.h \
             user_watcher.h \
             file_events_manager.h \
             cron_entry.h
SOURCES   += \
             logger.cc \
             process.cc \
             service.cc \
             service_config.cc \
             service_watcher.cc \
             user_watcher.cc \
             file_events_manager.cc \
             ../death_watch/death_watch.cc \
             ../notifications/notifications.cc \
             cron_entry.cc \
             service_spawner.cc
LIBS      += ../liblogger.a ../libcore.a ../libyajl.a ../libhiredis.a ../libquazip.a -lz
TARGET    = ../../bin/svdss
