
TEMPLATE = lib
CONFIG += qt warn_on staticlib
QT -= gui
DEPENDPATH += .
INCLUDEPATH += .

DEFINES += QUAZIP_BUILD
CONFIG(staticlib): DEFINES += QUAZIP_STATIC

HEADERS += crypt.h \
        ioapi.h \
        JlCompress.h \
        quaadler32.h \
        quachecksum32.h \
        quacrc32.h \
        quagzipfile.h \
        quaziodevice.h \
        quazipdir.h \
        quazipfile.h \
        quazipfileinfo.h \
        quazip_global.h \
        quazip.h \
        quazipnewinfo.h \
        unzip.h \
        zip.h

SOURCES += *.c *.cpp

TARGET = ../quazip
