# ServeD natives - Web App Deployer (WAD) - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../core/json_api.h \
             ../notifications/notifications.h \
             ../service_spawner/process.h \
             ../death_watch/death_watch.h \
             ../core/utils.h \
             ../core/logger.h \
             ../notifications/notifications.h \
             ../service_spawner/cron_entry.h \
             ../service_spawner/service_config.h \
             ../service_spawner/service.h \
             webapp_types.h \
             deploy.h
SOURCES   += ../core/json_api.cc \
             ../service_spawner/process.cc \
             ../core/utils.cc \
             ../death_watch/death_watch.cc \
             ../core/logger.cc \
             ../notifications/notifications.cc \
             ../service_spawner/cron_entry.cc \
             ../service_spawner/service_config.cc \
             ../service_spawner/service.cc \
             webapp_types.cc \
             deploy.cc \
             deployer.cc
LIBS      += -lz ../libyajl.a ../liblogger.a ../libhiredis.a ../libquazip.a
TARGET    = ../../bin/svddeployer
