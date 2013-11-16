/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "logger.h"


void ConsoleLoggerTimer::invokeTrigger() {
    QString dir = getHomeDir();
    if (QFile::exists(dir + "/.warn")) {
        logInfo() << "Invoked logger level change to level 'warning'.";
        QFile::remove(dir + "/.warn");
        logger->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
        logger->setDetailsLevel(Logger::Warning);
    }

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


ConsoleLoggerTimer::ConsoleLoggerTimer(ConsoleAppender *appender) {
    this->logger = appender;
    QTimer *timer = new QTimer(this);
    timer->setInterval(ONE_SECOND_OF_DELAY / 1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(invokeTrigger()));
    timer->start();
}


void FileLoggerTimer::invokeTrigger() {
    QString dir = getHomeDir();
    if (QFile::exists(dir + "/.warn")) {
        logInfo() << "Invoked logger level change to level 'warning'.";
        QFile::remove(dir + "/.warn");
        logger->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
        logger->setDetailsLevel(Logger::Warning);
    }

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


FileLoggerTimer::FileLoggerTimer(FileAppender *appender) {
    this->logger = appender;
    QTimer *timer = new QTimer(this);
    timer->setInterval(ONE_SECOND_OF_DELAY / 1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(invokeTrigger()));
    timer->start();
}
