# ServeD natives - ServeD Distribution Spell - © 2013-2014 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(../Common.pro)

QT += core network

TARGET = ../../bin/svddispel

DEFINES += NZMQT_LIB

HEADERS += \
        nzmqt/*.hpp \
        ../service_spawner/utils.h \
        ../notifications/notifications.h \
        ../quazip/quazip.h \
        ../cutelogger/AbstractAppender.h \
        *.h

SOURCES += \
        ../service_spawner/utils.cc \
        ../notifications/notifications.cc \
        nzmqt/*.cpp \
        dispel_core.cc \
        dispel_publisher.cc \
        dispel_subscriber.cc \
        dispel.cc

QMAKE_CXXFLAGS += -w

LIBS += \
        ../libzeromq.a \
        ../liblogger.a \
        ../libquazip.a \
        -lz
