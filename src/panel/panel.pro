# ServeD natives - Control Panel
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../service_spawner/service.h \
             ../death_watch/death_watch.h \
             ../service_spawner/process.h \
             ../service_spawner/service_config.h \
             ../service_spawner/utils.h \
             ../jsoncpp/json/json.h \
             ../cutelogger/Logger.h \
             ../cutelogger/ConsoleAppender.h \
             ../cutelogger/FileAppender.h \
             ../service_spawner/cron_entry.h \
             ../service_spawner/logger.h \
             panel.h

SOURCES   += ../service_spawner/service.cc \
             ../service_spawner/process.cc \
             ../service_spawner/service_config.cc \
             ../service_spawner/utils.cc \
             ../death_watch/death_watch.cc \
             ../service_spawner/cron_entry.cc \
             ../service_spawner/logger.cc \
             panel.cc

mac {
      LIBS      += ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a -lz -lncurses
} else {
      LIBS      += ../libjsoncpp.a ../liblogger.a ../libhiredis.a ../libquazip.a -lz -lncursesw
}
TARGET    = ../../svdpanel
