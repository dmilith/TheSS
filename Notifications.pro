# ServeD natives - Notifications System Manager - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/notifications

notifications.depends = src/hiredis src/cutelogger src/quazip src/yajl src/irc
