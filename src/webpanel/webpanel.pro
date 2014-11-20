# ServeD natives - Web Panel
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += network websockets
HEADERS   += \
            webpanel.h

SOURCES   += \
            webpanel.cc \
            webpanel_spawner.cc

LIBS += ../liblogger.a ../libyajl.a ../libcore.a ../libquazip.a -lz
# unix:mac {
#       LIBS += -lncurses
# } else {
#       LIBS += -lncursesw
# }
TARGET    = ../../bin/svdwebpanel
