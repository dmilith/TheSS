# ServeD natives - Coreginx Helper - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/coreginx_helper
CONFIG += ordered

coreginx_helper.depends = src/quazip src/cutelogger src/hiredis src/yajl src/notifications src/death_watch src/deployer
