# ServeD natives - Web App Deployer (WAD) - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../service_spawner/service.h \
             ../notifications/notifications.h \
             ../death_watch/death_watch.h \
             ../service_spawner/process.h \
             ../service_spawner/data_collector.h \
             ../service_spawner/service_config.h \
             ../service_spawner/utils.h \
             ../service_spawner/service_watcher.h \
             ../service_spawner/user_watcher.h \
             ../service_spawner/file_events_manager.h \
             ../jsoncpp/json/json.h \
             ../cutelogger/Logger.h \
             ../cutelogger/ConsoleAppender.h \
             ../cutelogger/FileAppender.h \
             ../service_spawner/cron_entry.h \
             ../service_spawner/logger.h \
             ../notifications/notifications.h \
             webapp_types.h \
             deployer.h
SOURCES   += ../service_spawner/service.cc \
             ../service_spawner/data_collector.cc \
             ../service_spawner/process.cc \
             ../service_spawner/service_config.cc \
             ../service_spawner/utils.cc \
             ../service_spawner/service_watcher.cc \
             ../service_spawner/user_watcher.cc \
             ../service_spawner/file_events_manager.cc \
             ../death_watch/death_watch.cc \
             ../service_spawner/cron_entry.cc \
             ../service_spawner/logger.cc \
             ../notifications/notifications.cc \
             webapp_types.cc \
             deployer.cc
LIBS      += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../libquazip.a -lz
TARGET    = ../../bin/svddeployer
