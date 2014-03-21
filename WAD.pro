# ServeD natives - Web App Deployer (WAD) - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/deployer

deployer.depends = src/cutelogger src/quazip src/hiredis src/yajl src/notifications src/death_watch src/service_spawner