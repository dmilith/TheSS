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
             ../notifications/notifications.h \
             tail.h \
             panel.h \
             gui.h \
             service.h

SOURCES   += ../service_spawner/utils.cc \
             ../service_spawner/logger.cc \
             ../service_spawner/process.cc \
             ../service_spawner/file_events_manager.cc \
             ../service_spawner/service_config.cc \
             ../notifications/notifications.cc \
             main.cc \
             panel.cc \
             gui.cc \
             scroll_list.cc \
             service.cc \
             tail.cc \
             ansi.cc

mac {
      LIBS      += ../liblogger.a ../libquazip.a ../libyail.a -lz -lncurses
} else {
      LIBS      += ../liblogger.a ../libquazip.a ../libyail.a -lz -lncursesw
}
TARGET    = ../../bin/svdpanel
