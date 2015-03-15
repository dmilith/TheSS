/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "logger-core.h"


void LoggerTimer::invokeTrigger() {
    QString dir = getHomeDir();
    if (QFile::exists(dir + "/.warn")) {
        logInfo() << "Invoked logger level change to level 'warning'.";
        QFile::remove(dir + "/.warn");
        logger->setLoggingLevel(WarnLevel);
    }

    if (QFile::exists(dir + "/.info")) {
        logInfo() << "Invoked logger level change to level 'info'.";
        QFile::remove(dir + "/.info");
        logger->setLoggingLevel(InfoLevel);
    }

    if (QFile::exists(dir + "/.debug")) {
        logInfo() << "Invoked logger level change to level 'debug'.";
        QFile::remove(dir + "/.debug");
        logger->setLoggingLevel(DebugLevel);
    }

    if (QFile::exists(dir + "/.trace")) {
        logInfo() << "Invoked logger level change to level 'trace'.";
        QFile::remove(dir + "/.trace");
        logger->setLoggingLevel(TraceLevel);
    }
}


LoggerTimer::LoggerTimer(Logger* appender) {
    this->logger = appender;
    QTimer *timer = new QTimer(this);
    timer->setInterval(ONE_SECOND_OF_DELAY / 1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(invokeTrigger()));
    timer->start();
}
