# ServeD natives - Death Watch (reliable kill replacement)
#

include(../Common.pro)

TEMPLATE = app

SOURCES += death_watch.cc \
           ../core/utils-core.cc \
           death_watcher.cc

HEADERS += ../core/utils-core.h \
           death_watch.h

LIBS      += ../liblogger.a
TARGET    = ../../bin/svddw
