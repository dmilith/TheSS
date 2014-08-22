# ServeD natives - ServeD Shell with full PTY support - © 2013-2014 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#   Michał (tallica) Lipski
#

include(../Common.pro)


HEADERS   += \
            ../globals/globals.h \
            shellutils.h

SOURCES   += \
            shellutils.cc \
            shell.cc

QMAKE_CXXFLAGS += -w
LIBS      += -lz -lutil -lprocstat
unix:mac {
    LIBS -= -lutil -lprocstat
}
unix:linux-* {
    LIBS -= -lprocstat
}
TARGET    = ../../bin/svdshell
