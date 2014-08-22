# ServeD natives - Coreginx Helper - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/cutelogger src/core
CONFIG += ordered

coreginx_helper.depends = src/cutelogger src/core src/quazip src/hiredis src/yajl src/notifications
