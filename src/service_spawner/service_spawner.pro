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
             data_collector.h \
             service_config.h \
             utils.h \
             service_watcher.h \
             user_watcher.h \
             file_events_manager.h \
             ../jsoncpp/json/json.h \
             ../cutelogger/Logger.h \
             ../cutelogger/ConsoleAppender.h \
             ../cutelogger/FileAppender.h \
             cron_entry.h \
             logger.h \
             ../notifications/notifications.h
SOURCES   += service.cc \
             data_collector.cc \
             process.cc \
             service_config.cc \
             utils.cc \
             service_watcher.cc \
             user_watcher.cc \
             file_events_manager.cc \
             ../death_watch/death_watch.cc \
             service_spawner.cc \
             cron_entry.cc \
             logger.cc
             # ../notifications/notifications.cc
LIBS      += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../libquazip.a -lz
TARGET    = ../../bin/svdss
