
include(../Common.pro)

CONFIG += testlib
QT -= gui
QT += testlib network
TEMPLATE = app
TARGET = ../../bin/test-app

DEFINES += THESS_TEST_MODE

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += TestLibrary.h \
           ../yajl/api/yajl_tree.h \
           ../globals/globals.h \
           ../service_spawner/service_config.h \
           ../cutelogger/Logger.h \
           ../cutelogger/ConsoleAppender.h \
           ../cutelogger/FileAppender.h \
           ../service_spawner/service.h \
           ../service_spawner/process.h \
           ../notifications/notifications.h \
           ../deployer/webapp_types.h
SOURCES += \
           ../service_spawner/service_config.cc \
           ../service_spawner/process.cc \
           ../service_spawner/service.cc \
           ../notifications/notifications.cc \
           ../deployer/webapp_types.cc \
           TestLibrary.cc

LIBS += -lz ../liblogger.a ../libcore.a ../libhiredis.a ../libyajl.a ../libquazip.a
