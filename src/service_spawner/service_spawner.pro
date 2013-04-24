# ServeD natives - TheSS - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += service.h \
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
             webapp_deployer.h \
             webapp_types.h \
             webapp_watcher.h \
             logger.h
SOURCES   += service.cc \
             data_collector.cc \
             process.cc \
             service_config.cc \
             utils.cc \
             service_watcher.cc \
             user_watcher.cc \
             file_events_manager.cc \
             webapp_deployer.cc \
             webapp_types.cc \
             webapp_watcher.cc \
             ../death_watch/death_watch.cc \
             service_spawner.cc \
             logger.cc
LIBS      += ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a -lz
TARGET    = ../../svdss
