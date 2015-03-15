# ServeD natives - Web App Deployer (WAD) - (c) 2o13-2o15 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/qslog src/core src/hiredis src/deployer
# CONFIG += ordered

deployer.depends = src/qslog src/core src/hiredis src/service_spawner
