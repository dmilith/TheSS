# ServeD natives - IRC notificator
#

include(../Common.pro)

TEMPLATE = app
QT += network

SOURCES += irc_notify.cc \
           ../service_spawner/utils.cc \
           ../service_spawner/logger.cc

HEADERS += ../service_spawner/logger.h

LIBS      += ../libirc.a ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a -lz
TARGET    = ../../bin/svdirc_notify
