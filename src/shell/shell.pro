# ServeD natives - ServeD Shell with full PTY support - © 2013-2014 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#   Michał (tallica) Lipski
#

include(../Common.pro)


HEADERS   += \
            ../core/core.h \
            ../globals/globals.h \
            shellutils.h

SOURCES   += \
            ../core/core.cc \
            shellutils.cc \
            shell.cc

QMAKE_CXXFLAGS += -w
LIBS      += -lz
unix:!mac {
  LIBS += -lutil
}
TARGET    = ../../bin/svdshell
