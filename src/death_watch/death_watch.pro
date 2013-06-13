# ServeD natives - Death Watch (reliable kill replacement)
#

include(../Common.pro)

TEMPLATE = app

SOURCES += death_watch.cc \
           death_watcher.cc

HEADERS += death_watch.h

LIBS      += ../liblogger.a
TARGET    = ../../bin/svddw
