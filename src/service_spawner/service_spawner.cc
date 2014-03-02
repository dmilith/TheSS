/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include <QtCore>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "logger.h"
#include "service_config.h"
#include "service_watcher.h"
#include "user_watcher.h"
#include "utils.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    QStringList args = app.arguments();
    bool background = true; /* by default launch svdss as daemon */
    QRegExp rxEnableForeground("-f"); /* run in foreground */
    QRegExp rxEnableDebug("-d");
    QRegExp rxEnableTrace("-t");
    QRegExp rxPrintVersion("-v");
    uint uid = getuid();

    bool debug = false, trace = false;
    for (int i = 1; i < args.size(); ++i) {
        if (rxEnableDebug.indexIn(args.at(i)) != -1 ) {
            debug = true;
        }
        if (rxEnableForeground.indexIn(args.at(i)) != -1 ) {
            background = false;
        }
        if (rxEnableTrace.indexIn(args.at(i)) != -1 ) {
            debug = true;
            trace = true;
        }
        if (rxPrintVersion.indexIn(args.at(i)) != -1) {
            cout << "ServeD Service Spawner v" << APP_VERSION << ". " << COPYRIGHT << endl;
            return EXIT_SUCCESS;
        }
    }

    /* Logger setup */
    if (not background) {
        ConsoleAppender *consoleAppender = new ConsoleAppender();
        Logger::registerAppender(consoleAppender);
        consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
        if (trace && debug)
            consoleAppender->setDetailsLevel(Logger::Trace);
        else if (debug && !trace)
            consoleAppender->setDetailsLevel(Logger::Debug);
        else {
            consoleAppender->setDetailsLevel(Logger::Info);
            consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
        }
        new ConsoleLoggerTimer(consoleAppender);

    } else {
        FileAppender *fileAppender;
        if (getuid() == 0)
            fileAppender = new FileAppender(QString(SYSTEM_USERS_DIR) + DEFAULT_SS_LOG_FILE);
        else
            fileAppender = new FileAppender(QString(getenv("HOME")) + DEFAULT_SS_LOG_FILE);

        Logger::registerAppender(fileAppender);
        fileAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
        if (trace && debug)
            fileAppender->setDetailsLevel(Logger::Trace);
        else if (debug && !trace)
            fileAppender->setDetailsLevel(Logger::Debug);
        else {
            fileAppender->setDetailsLevel(Logger::Info);
            fileAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
        }
        new FileLoggerTimer(fileAppender);
    }

    /* file lock setup */
    QString lockName = getHomeDir() + "/." + getenv("USER") + ".pid";
    if (getuid() == 0) {
        lockName = getHomeDir() + "/.root.pid";
    }
    if (QFile::exists(lockName)) {
        bool ok;
        QString aPid = readFileContents(lockName).trimmed();
        uint pid = aPid.toInt(&ok, 10);
        if (ok) {
            if (pidIsAlive(pid) or pid == 0) { /* NOTE: if pid == 0 it means that SS is runned from SS root maintainer */
                logError() << "Service Spawner is already running.";
                return LOCK_FILE_OCCUPIED_ERROR; /* can not open */
            } else
                logDebug() << "No alive Service Spawner pid found";

        } else {
            logWarn() << "Pid file is damaged or doesn't contains valid pid. File will be removed";
            QFile::remove(lockName);
        }
    }
    logDebug() << "Lock name:" << lockName;
    writeToFile(lockName, QString::number(getpid()), false); /* get process pid and record it to pid file no logrotate */

    signal(SIGINT, unixSignalHandler);
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */
    notification("Launching TheSS v" + QString(APP_VERSION) + " on host: " + QHostInfo::localHostName() + " for uid: " + QString::number(uid));

    if (uid == 0) {
        logInfo("Root Mode Service Spawner v" + QString(APP_VERSION) + ". " + QString(COPYRIGHT));
        setPublicDirPriviledges(getOrCreateDir(DEFAULT_PUBLIC_DIR));
        setupDefaultVPNNetwork();

        /* Setting up root watchers */
        new SvdUserWatcher();

    } else {
        logInfo("Service Spawner v" + QString(APP_VERSION) + ". " + QString(COPYRIGHT));
        logDebug() << "Spawning for user:" << getenv("USER");

        logDebug() << "Checking user directory priviledges";
        setUserDirPriviledges(getHomeDir());

        /* Setting up user watchers */
        new SvdUserWatcher();
    }

    return app.exec();
}