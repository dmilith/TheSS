# TheSS Common - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#


QT -= gui
QTDIR += /Software/Qtws53 /Software/Qtbase53 /Software/Qt53 /Software/Qt
CONFIG += console
CONFIG -= app_bundle
OBJECTS_DIR = ../.obj
MOC_DIR = ../.moc
DEFINES += THESS_NORMAL_MODE

QMAKE_CXX = clang++
QMAKE_CC = clang
QMAKE_CPP = clang++ -E
QMAKE_LINK = clang++

QMAKE_CFLAGS += -fcolor-diagnostics -Wself-assign -fPIC -fPIE
QMAKE_CXXFLAGS += -fcolor-diagnostics -Wself-assign -fPIC -fPIE


mac {
    # development opts:
    QMAKE_CC = ccache clang
    QMAKE_CPP = ccache clang++ -E
    QMAKE_CXX = ccache clang++ -w
    QMAKE_LINK = ccache clang++
    QMAKE_CFLAGS += -O0 -w -gline-tables-only
    QMAKE_CXXFLAGS += -O0 -gline-tables-only -std=c++11
    DEFINES += QT_DEBUG
}


freebsd {
    # production opts:
    QMAKE_CFLAGS += -Os -w
    QMAKE_CXXFLAGS += -Os -w
    DEFINES += NDEBUG QT_NO_DEBUG
}


# linux {
#     DEFINES += NDEBUG QT_NO_DEBUG
#     QMAKE_CXX = g++
#     QMAKE_CC = gcc
#     QMAKE_CPP = cpp
#     QMAKE_LINK = g++
#     QMAKE_CFLAGS += -fPIC -fPIE -Os -std=c++0x
#     QMAKE_CXXFLAGS += -fPIC -fPIE -Os -std=c++0x
# }
