# ServeD natives - TheSS - (c) 2o13-2o15 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/qslog src/core src/quazip src/hiredis src/yajl src/test
CONFIG += ordered

service_spawner.depends = src/qslog src/core src/quazip src/hiredis src/yajl src/notifications src/death_watch src/test
