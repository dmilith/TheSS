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
             ../notifications/notifications.h \
             ../deployer/deploy.h

SOURCES   += ../service_spawner/process.cc \
             ../service_spawner/utils.cc \
             ../death_watch/death_watch.cc \
             ../service_spawner/logger.cc \
             ../notifications/notifications.cc \
             ../deployer/webapp_types.cc \
             ../deployer/deploy.cc \
             coreginx_helper.cc

LIBS      += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../libquazip.a -lz
TARGET    = ../../bin/svdcoreginx_helper
