# Syndir Common - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#


QT -= gui

QMAKE_CXX = clang++
QMAKE_CC = clang

QMAKE_CXXFLAGS += -fcolor-diagnostics -Qunused-arguments -Wself-assign -fPIC -fPIE -Os -wunused-variable

mac {

  QMAKE_CXXFLAGS  += -std=c++11

} else {

  CONFIG += link_pkgconfig
  PKGCONFIG = QtCore

}

