# ServeD natives - TheSS - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/cutelogger src/quazip src/hiredis src/yajl src/service_spawner src/test

service_spawner.depends = src/cutelogger src/quazip src/hiredis src/yajl src/notifications src/death_watch src/service_spawner
