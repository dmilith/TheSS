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
        ../notifications/notifications.h \
        ../quazip/quazip.h \
        ../cutelogger/AbstractAppender.h \
        *.h

SOURCES += \
        ../notifications/notifications.cc \
        nzmqt/*.cpp \
        dispel_core.cc \
        dispel_publisher.cc \
        dispel_subscriber.cc \
        dispel.cc

QMAKE_CXXFLAGS += -w

LIBS += \
        ../libcore.a \
        ../libzeromq.a \
        ../liblogger.a \
        ../libquazip.a \
        ../libyajl.a \
        -lz
