# ServeD natives - ServeD core library - © 2013-2014 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

TEMPLATE = lib
CONFIG += qt warn_off staticlib
QT -= gui

HEADERS   += \
            ../globals/globals.h \
            utils-core.h \
            utils.h \
            json_api.h \
            core.h

SOURCES   += \
            utils-core.cc \
            utils.cc \
            json_api.cc \
            core.cc

QMAKE_CXXFLAGS += -w
LIBS      += -lz ../liblogger.a
TARGET    = ../core
