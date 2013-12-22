# ServeD natives - Coreginx Helper - © 2013 verknowsys.com
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
             ../service_spawner/file_events_manager.h \
             ../notifications/notifications.h \
             ../deployer/deploy.h \
             public_watcher.h

SOURCES   += ../service_spawner/process.cc \
             ../service_spawner/utils.cc \
             ../death_watch/death_watch.cc \
             ../service_spawner/logger.cc \
             ../notifications/notifications.cc \
             ../service_spawner/file_events_manager.cc \
             ../deployer/webapp_types.cc \
             ../deployer/deploy.cc \
             public_watcher.cc \
             coreginx_helper.cc

LIBS      += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../libquazip.a -lz
TARGET    = ../../bin/svdcoreginx_helper
