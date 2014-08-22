/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __SVDLOGGER_H__
#define __SVDLOGGER_H__

#include <QtCore>
#include "../core/utils.h"


class ConsoleLoggerTimer: public QObject {
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
