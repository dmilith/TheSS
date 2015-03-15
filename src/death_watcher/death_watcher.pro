# ServeD natives - Death Watch (reliable kill replacement)
#

include(../Common.pro)

TEMPLATE = app

SOURCES += death_watcher.cc

LIBS      += ../libcore.a ../libqslog.a
TARGET    = ../../bin/svddw
