# ServeD natives - Coreginx Helper - (c) 2o13-2o14 verknowsys.com
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
             ../core/utils-core.h \
             ../core/utils.h \
             ../core/logger.h \
             ../service_spawner/file_events_manager.h \
             ../notifications/notifications.h \
             ../service_spawner/cron_entry.h \
             ../service_spawner/service_config.h \
             ../service_spawner/service.h \
             public_watcher.h

SOURCES   += ../core/json_api.cc \
             ../service_spawner/process.cc \
             ../core/utils-core.cc \
             ../core/utils.cc \
             ../death_watch/death_watch.cc \
             ../core/logger.cc \
             ../notifications/notifications.cc \
             ../service_spawner/file_events_manager.cc \
             ../deployer/webapp_types.cc \
             ../service_spawner/cron_entry.cc \
             ../service_spawner/service_config.cc \
             ../service_spawner/service.cc \
             public_watcher.cc \
             coreginx_helper.cc

LIBS      += ../libyajl.a ../liblogger.a ../libhiredis.a ../libquazip.a -lz
TARGET    = ../../bin/svdcoreginx_helper
