# ServeD natives - Control Panel
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network
HEADERS   += \
            ../service_spawner/service_config.h \
            ../service_spawner/process.h \
            ../notifications/notifications.h \
            tail.h \
            panel.h \
            panel_service.h \
            gui.h

SOURCES   += \
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

LIBS += ../libcore.a ../libqslog.a ../libquazip.a ../libyajl.a -lz
unix:mac {
      LIBS += -lncurses
} else {
      LIBS += -lncursesw
}
TARGET    = ../../bin/svdpanel
