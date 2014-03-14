
include(../Common.pro)

CONFIG += qtestlib
QT -= gui
QT += network
TEMPLATE = app
TARGET = ../../bin/test-app

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += TestLibrary.h \
           ../yajl/api/yajl_tree.h \
           ../globals/globals.h \
           ../service_spawner/service_config.h \
           ../service_spawner/utils.h \
           ../cutelogger/Logger.h \
           ../cutelogger/ConsoleAppender.h \
           ../cutelogger/FileAppender.h \
           ../service_spawner/service.h \
           ../service_spawner/process.h \
           ../death_watch/death_watch.h \
           ../notifications/notifications.h \
           ../service_spawner/cron_entry.h \
           ../deployer/webapp_types.h \
           ../service_spawner/cron_entry.h
SOURCES += ../service_spawner/service_config.cc \
           ../service_spawner/utils.cc \
           ../service_spawner/process.cc \
           ../service_spawner/service.cc \
           ../death_watch/death_watch.cc \
           ../service_spawner/cron_entry.cc \
           ../notifications/notifications.cc \
           ../deployer/webapp_types.cc \
           TestLibrary.cc

LIBS += ../libyajl.a ../liblogger.a ../libquazip.a -lz
