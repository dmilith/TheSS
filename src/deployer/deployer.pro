# ServeD natives - Web App Deployer (WAD) - (c) 2o13-2o15 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network websockets
HEADERS   += ../notifications/notifications.h \
             ../service_spawner/process.h \
             ../notifications/notifications.h \
             ../service_spawner/service_config.h \
             ../service_spawner/service.h \
             ../service_spawner/api.h \
             webapp_types.h \
             deploy.h
SOURCES   += ../service_spawner/process.cc \
             ../notifications/notifications.cc \
             ../service_spawner/service_config.cc \
             ../service_spawner/service.cc \
             ../service_spawner/api.cc \
             webapp_types.cc \
             deploy.cc \
             deployer.cc

LIBS      += -lz ../libcore.a ../liblogger.a ../libyajl.a ../libquazip.a
TARGET    = ../../bin/svddeployer
