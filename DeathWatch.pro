# ServeD natives - Process DeathWatch - (c) 2o13-2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/core src/death_watcher

notifications.depends = src/cutelogger src/core
