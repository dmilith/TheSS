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
             panel.h

SOURCES   += ../service_spawner/utils.cc \
             ../service_spawner/logger.cc \
             panel.cc

mac {
      LIBS      += ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncurses
} else {
      LIBS      += ../liblogger.a ../libquazip.a ../libjsoncpp.a -lz -lncursesw
}
TARGET    = ../../svdpanel
