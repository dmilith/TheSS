# ServeD natives - Control Panel
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += ../service_spawner/utils.h \
             ../cutelogger/Logger.h \
             ../cutelogger/ConsoleAppender.h \
             ../cutelogger/FileAppender.h \
             ../service_spawner/logger.h \
             ../service_spawner/service_config.h \
             ../service_spawner/process.h \
             tail.h \
             panel.h \
             gui.h \
             service.h

SOURCES   += main.cc \
             panel.cc \
             gui.cc \
             scroll_list.cc \
             service.cc \
             tail.cc \
             ansi.cc

mac {
      LIBS      += -lz -lncurses ../liblogger.a ../libquazip.a ../libjsoncpp.a ../libnotifications.a ../service_spawner/.obj/utils.o ../service_spawner/.obj/logger.o ../service_spawner/.obj/process.o ../service_spawner/.obj/file_events_manager.o ../service_spawner/.obj/service_config.o
} else {
      LIBS      += -lz -lncursesw ../liblogger.a ../libquazip.a ../libjsoncpp.a ../libnotifications.a ../service_spawner/.obj/utils.o ../service_spawner/.obj/logger.o ../service_spawner/.obj/process.o ../service_spawner/.obj/file_events_manager.o ../service_spawner/.obj/service_config.o
}
TARGET    = ../../bin/svdpanel
