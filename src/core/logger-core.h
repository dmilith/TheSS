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
#include "../qslog/QsLog.h"
#include "../qslog/QsLogDest.h"

using namespace QsLogging;


class LoggerTimer: public QObject {
    Q_OBJECT

    private:
        Logger* logger;

    public:
        LoggerTimer(Logger* appender);

    public slots:
        void invokeTrigger();

};


// class FileLoggerTimer: QObject {
//     Q_OBJECT

//     private:
//         void *logger;

//     public:
//         FileLoggerTimer(void *appender);

//     public slots:
//         void invokeTrigger();

// };


#endif
