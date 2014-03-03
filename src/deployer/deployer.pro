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
SOURCES   += webapp_types.cc \
             deploy.cc \
             deployer.cc
LIBS      += -lz ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../service_spawner/.obj/cron_entry.o ../service_spawner/.obj/logger.o ../service_spawner/.obj/utils.o ../service_spawner/.obj/process.o ../service_spawner/.obj/service_config.o ../service_spawner/.obj/service.o ../notifications/.obj/notifications.o ../death_watch/.obj/death_watch.o
TARGET    = ../../bin/svddeployer
