# ServeD natives - Control Panel
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += \
            ../core/json_api.h \
            ../service_spawner/cron_entry.h \
            ../core/utils.h \
            ../cutelogger/Logger.h \
            ../cutelogger/ConsoleAppender.h \
            ../cutelogger/FileAppender.h \
            ../core/logger.h \
            ../service_spawner/service_config.h \
            ../service_spawner/process.h \
            ../notifications/notifications.h \
            tail.h \
            panel.h \
            panel_service.h \
            gui.h

SOURCES   += \
            ../core/json_api.cc \
            ../service_spawner/cron_entry.cc \
            ../core/utils.cc \
            ../core/logger.cc \
            ../service_spawner/process.cc \
            ../service_spawner/file_events_manager.cc \
            ../service_spawner/service_config.cc \
            ../notifications/notifications.cc \
            main.cc \
            panel.cc \
            gui.cc \
            scroll_list.cc \
            panel_service.cc \
            tail.cc \
            ansi.cc

mac {
      LIBS      += ../liblogger.a ../libquazip.a ../libyajl.a -lz -lncurses
} else {
      LIBS      += ../liblogger.a ../libquazip.a ../libyajl.a -lz -lncursesw
}
TARGET    = ../../bin/svdpanel
