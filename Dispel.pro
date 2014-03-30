# ServeD natives - ServeD Distributed Spell - © 2013 verknowsys.com
#
# author:
#   Daniel (dmilith) Dettlaff
#

include(src/Common.pro)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src/cutelogger src/quazip src/hiredis src/yajl src/notifications src/zeromq src/dispel

src/dispel.depends = src/zeromq src/notifications
