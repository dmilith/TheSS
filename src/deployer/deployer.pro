# ServeD natives - Web App Deployer (WAD) - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../notifications/notifications.h \
             ../service_spawner/process.h \
             ../death_watch/death_watch.h \
             ../service_spawner/utils.h \
             ../service_spawner/logger.h \
             ../notifications/notifications.h \
             ../service_spawner/cron_entry.h \
             ../service_spawner/service_config.h \
             ../service_spawner/service.h \
             webapp_types.h \
             deploy.h
SOURCES   += ../service_spawner/process.cc \
             ../service_spawner/utils.cc \
             ../death_watch/death_watch.cc \
             ../service_spawner/logger.cc \
             ../notifications/notifications.cc \
             ../service_spawner/cron_entry.cc \
             ../service_spawner/service_config.cc \
             ../service_spawner/service.cc \
             webapp_types.cc \
             deploy.cc \
             deployer.cc
LIBS      += ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../libquazip.a -lz
TARGET    = ../../bin/svddeployer
