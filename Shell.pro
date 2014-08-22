# ServeD Shell with full PTY support - © 2013-2014 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#   Michał (tallica) Lipski
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/shell
CONFIG += ordered

shell.depends = src/core
