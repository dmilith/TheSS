# NCurses control panel for TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/panel

panel.depends = src/jsoncpp src/cutelogger src/quazip src/notifications src/service_spawner
