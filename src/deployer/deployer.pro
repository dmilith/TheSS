# ServeD natives - Web App Deployer (WAD) - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../notifications/notifications.h \
             ../service_spawner/process.h \
             ../service_spawner/utils.h \
             ../service_spawner/logger.h \
             ../notifications/notifications.h \
             webapp_types.h \
             deployer.h
SOURCES   += ../service_spawner/process.cc \
             ../service_spawner/utils.cc \
             ../service_spawner/logger.cc \
             ../notifications/notifications.cc \
             webapp_types.cc \
             deployer.cc
LIBS      += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a ../libquazip.a -lz
TARGET    = ../../bin/svddeployer
