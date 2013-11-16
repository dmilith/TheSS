/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <QtCore>
#include "utils.h"


class ConsoleLoggerTimer: QObject {
    Q_OBJECT

    private:
        ConsoleAppender *logger;

    public:
        ConsoleLoggerTimer(ConsoleAppender *appender);

    public slots:
        void invokeTrigger();

};


class FileLoggerTimer: QObject {
    Q_OBJECT

    private:
        FileAppender *logger;

    public:
        FileLoggerTimer(FileAppender *appender);

    public slots:
        void invokeTrigger();

};


#endif
