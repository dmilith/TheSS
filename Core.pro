# ServeD natives - ServeD Core - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE = subdirs
SUBDIRS = src/qslog src/core

core.depends = src/qslog
