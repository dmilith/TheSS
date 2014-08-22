# ServeD natives - ServeD Core - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE = subdirs
SUBDIRS = src/cutelogger src/core

core.depends = src/cutelogger
