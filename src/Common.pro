# Syndir Common - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#


QT -= gui

QMAKE_CXX = clang++
QMAKE_CC = clang
QMAKE_CPP = clang++ -E

QMAKE_CXXFLAGS += -fcolor-diagnostics -Qunused-arguments -Wself-assign -fPIC -fPIE -O3 -wunused-variable

mac {

  QMAKE_CXXFLAGS  += -std=c++11

} else {

  QMAKE_CXXFLAGS  += -w
  CONFIG += link_pkgconfig
  PKGCONFIG = QtCore

}

