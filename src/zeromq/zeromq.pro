#
# The 0MQ lightweight messaging kernel is a library which extends the
# standard socket interfaces with features traditionally provided by
# specialised messaging middleware products. 0MQ sockets provide an
# abstraction of asynchronous message queues, multiple messaging patterns,
# message filtering (subscriptions), seamless access to multiple transport
# protocols and more.
#
# library version: 4.0.4

include(../Common.pro)

TARGET = ../zeromq
TEMPLATE = lib
CONFIG += staticlib

HEADERS   += *.hpp
SOURCES   += *.cpp

QMAKE_CXXFLAGS += -w

# LIBS        += -lrt -lpthread