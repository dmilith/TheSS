# TheSS Common - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#


QT -= gui

QMAKE_CXX = clang++
QMAKE_CC = clang
QMAKE_CPP = clang++ -E
OBJECTS_DIR = ../.obj
MOC_DIR = ../.moc
DEFINES += THESS_NORMAL_MODE

mac {

  # development opts:
  QMAKE_CFLAGS += -fcolor-diagnostics -Wself-assign -fPIC -O0 -w -gline-tables-only
  QMAKE_CXXFLAGS += -fcolor-diagnostics -Wself-assign -fPIC -O0 -gline-tables-only -std=c++11
  DEFINES += QT_DEBUG

} else {

  # production opts:
  CONFIG += link_pkgconfig
  PKGCONFIG += QtCore

  DEFINES += NDEBUG QT_NO_DEBUG
  QMAKE_CFLAGS += -fcolor-diagnostics -Wself-assign -fPIC -fPIE -Os -w
  QMAKE_CXXFLAGS += -fcolor-diagnostics -Wself-assign -fPIC -fPIE -Os -w

}
