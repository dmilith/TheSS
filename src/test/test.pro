
include(../Common.pro)

CONFIG += qtestlib
QT -= gui
QT += network
TEMPLATE = app
TARGET = ../../bin/test-app

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += TestLibrary.h \
           ../jsoncpp/json/json.h \
           ../globals/globals.h \
           ../service_spawner/service_config.h \
           ../service_spawner/utils.h \
           ../cutelogger/Logger.h \
           ../cutelogger/ConsoleAppender.h \
           ../cutelogger/FileAppender.h \
           ../service_spawner/service.h \
           ../service_spawner/process.h \
           ../death_watch/death_watch.h \
           ../deployer/webapp_deployer.h \
           ../service_spawner/cron_entry.h \
           ../deployer/webapp_types.h \
           ../service_spawner/cron_entry.h
SOURCES += ../service_spawner/service_config.cc \
           ../service_spawner/utils.cc \
           ../service_spawner/process.cc \
           ../service_spawner/service.cc \
           ../death_watch/death_watch.cc \
           ../service_spawner/cron_entry.cc \
           ../deployer/webapp_deployer.cc \
           ../deployer/webapp_types.cc \
           TestLibrary.cc

LIBS += ../libnotifications.a ../libjsoncpp.a ../liblogger.a ../libquazip.a -lz
