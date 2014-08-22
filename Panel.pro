# NCurses control panel for TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/panel
CONFIG += ordered

panel.depends = src/core src/yajl src/cutelogger src/quazip src/notifications src/service_spawner
