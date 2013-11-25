# ServeD natives - IRC notificator
#

include(../Common.pro)

TEMPLATE = app
QT += network

SOURCES += irc_notify.cc \
           ../irc/irc.cc \
           ../service_spawner/utils.cc \
           ../service_spawner/logger.cc

HEADERS += ../irc/irc.h \
           ../service_spawner/logger.h

LIBS      += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a -lz
TARGET    = ../../bin/svdirc_notify
