# ServeD natives - Notifications System Manager - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/hiredis src/notifications
CONFIG += ordered

notifications.depends = src/core src/cutelogger src/quazip src/yajl src/hiredis
