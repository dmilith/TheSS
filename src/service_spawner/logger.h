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


class LoggerTimer: QObject {
    Q_OBJECT

    private:
        ConsoleAppender *logger;

    public:
        LoggerTimer(ConsoleAppender *appender);

    public slots:
        void invokeTrigger();

};

#endif
