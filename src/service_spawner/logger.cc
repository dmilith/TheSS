/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "logger.h"


void LoggerTimer::invokeTrigger() {
    QString dir = getHomeDir();
    if (QFile::exists(dir + "/.info")) {
        logInfo() << "Invoked logger level change to level 'info'.";
        QFile::remove(dir + "/.info");
        logger->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
        logger->setDetailsLevel(Logger::Info);
    }

    if (QFile::exists(dir + "/.debug")) {
        logInfo() << "Invoked logger level change to level 'debug'.";
        QFile::remove(dir + "/.debug");
        logger->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
        logger->setDetailsLevel(Logger::Debug);
    }

    if (QFile::exists(dir + "/.trace")) {
        logInfo() << "Invoked logger level change to level 'trace'.";
        QFile::remove(dir + "/.trace");
        logger->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
        logger->setDetailsLevel(Logger::Trace);
    }
}


LoggerTimer::LoggerTimer(ConsoleAppender *appender) {
    this->logger = appender;
    QTimer *timer = new QTimer(this);
    timer->setInterval(ONE_SECOND_OF_DELAY / 1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(invokeTrigger()));
    timer->start();
}
