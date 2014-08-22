# ServeD natives - ServeD Distributed Spell - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src/quazip src/yajl src/zeromq src/dispel

dispel.depends = src/core src/zeromq src/notifications
