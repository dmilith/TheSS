# Web control panel for TheSS - (c) 2o14 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE           = subdirs
SUBDIRS            = src/webpanel
CONFIG += ordered

panel.depends = src/yajl src/cutelogger
