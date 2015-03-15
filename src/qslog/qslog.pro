include(../Common.pro)

DEFINES += QS_LOG_LINE_NUMBERS    # automatically writes the file and line for each log message
#DEFINES += QS_LOG_DISABLE         # logging code is replaced with a no-op
DEFINES += QS_LOG_SEPARATE_THREAD # messages are queued and written from a separate thread
SOURCES += QsLogDest.cpp \
    QsLog.cpp \
    QsLogDestConsole.cpp \
    QsLogDestFile.cpp \
    QsLogDestFunctor.cpp

HEADERS += QsLogDest.h \
    QsLog.h \
    QsLogDestConsole.h \
    QsLogLevel.h \
    QsLogDestFile.h \
    QsLogDisableForThisFile.h \
    QsLogDestFunctor.h

OTHER_FILES += \
    QsLogChanges.txt \
    QsLogReadme.txt \
    LICENSE.txt

TARGET = ../qslog
TEMPLATE = lib
CONFIG += staticlib
