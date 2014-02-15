# ServeD natives - TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/cutelogger src/quazip src/hiredis src/jsoncpp src/service_spawner

src/service_spawner.depends = src/cutelogger src/quazip src/hiredis src/jsoncpp src/notifications src/death_watch
