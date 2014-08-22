/**
 *  @author dmilith
 *
 *   © 2013-2014 - VerKnowSys
 *
 */


#include "../deployer/deploy.h"
#include "public_watcher.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    setDefaultEncoding();
    QStringList args = app.arguments();
    bool background = true;
    QRegExp rxEnableForeground("-f"); /* run in foreground */
    QRegExp rxEnableDebug("-d");
    QRegExp rxEnableTrace("-t");
    QRegExp rxPrintVersion("-v");

    /* web app name is also service name, and git repository name */
    QRegExp rxWebAppName("-n");
    QRegExp rxWebDomain("-o");
    QRegExp rxWebStage("-s");
    QRegExp rxWebBranch("-b");

    QStringList errors, warnings;

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
            cout << "ServeD Coreginx Helper v" << APP_VERSION << ". " << COPYRIGHT << endl;
            return EXIT_SUCCESS;
        }
    }

    /* Logger setup */
    if (not background) {
        ConsoleAppender *consoleAppender = new ConsoleAppender();
        Logger::registerAppender(consoleAppender);
        consoleAppender->setFormat("%t{dd-HH:mm:ss} cgx[%-7l] <%c:(%F:%i)> %m\n");
        if (trace && debug)
            consoleAppender->setDetailsLevel(Logger::Trace);
        else if (debug && !trace)
            consoleAppender->setDetailsLevel(Logger::Debug);
        else {
            consoleAppender->setDetailsLevel(Logger::Info);
            consoleAppender->setFormat("%t{dd-HH:mm:ss} cgx[%-7l] %m\n");
        }
        new ConsoleLoggerTimer(consoleAppender);

    } else {
        FileAppender *fileAppender;
        fileAppender = new FileAppender(QString(SYSTEM_USERS_DIR) + DEFAULT_SS_LOG_FILE);

        Logger::registerAppender(fileAppender);
        fileAppender->setFormat("%t{dd-HH:mm:ss} cgx[%-7l] <%c:(%F:%i)> %m\n");
        if (trace && debug)
            fileAppender->setDetailsLevel(Logger::Trace);
        else if (debug && !trace)
            fileAppender->setDetailsLevel(Logger::Debug);
        else {
            fileAppender->setDetailsLevel(Logger::Info);
            fileAppender->setFormat("%t{dd-HH:mm:ss} cgx[%-7l] %m\n");
        }
        // new FileLoggerTimer(fileAppender);
    }

    /* mem lock setup */
    auto startTimer = QDateTime::currentMSecsSinceEpoch() / 5000;
    auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
    hash->addData(QString::number(startTimer).toUtf8(), QString::number(startTimer).length());
    auto shaId = hash->result().toHex();
    delete hash;
    const QString key = shaId.left(25) + ":" + QString::number(getuid());
    QSharedMemory memLock(key);
    if (not memLock.create(1)) {
        logError() << "Memory locked! You cannot launch more than one process per 5 seconds. (key: " + key + ")";
        logError() << "Internal cause:" << memLock.errorString();
        return EXIT_SUCCESS;
    }

    /* NOTE: make sure TheSS is running for user, and launch it if it's not: */
    bool ok = false;
    QString cgxhPidFile = getHomeDir() + "/.coreginx_helper.pid";
    QString aPid = readFileContents(cgxhPidFile).trimmed();
    uint pid = aPid.toInt(&ok, 10);
    if (ok) {
        if (pidIsAlive(pid)) {
            logError() << "Coreginx Helper is already running.";
            return LOCK_FILE_OCCUPIED_ERROR; /* can not open */
        }
    } else {
        QFile::remove(cgxhPidFile);
    }

    /* print warnings and errors */
    if (not warnings.isEmpty()) {
        Q_FOREACH(QString warning, warnings) {
            logWarn() << warning;
        }
    }

    if (not errors.isEmpty()) {
        Q_FOREACH(QString error, errors) {
            logError() << error;
        }
        raise(SIGTERM);
    }


    logInfo() << "Performing sanity checks..";
    if (getuid() != 0) {
        logError() << "Root account is necessary for Coreginx Helper.";
        raise(SIGTERM);
    }

    auto coreginxDataDir = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR + "/Coreginx";
    getOrCreateDir(coreginxDataDir);
    touch(coreginxDataDir + AUTOSTART_TRIGGER_FILE);
    if (not QFile::exists(coreginxDataDir + DEFAULT_SERVICE_RUNNING_FILE)) {
        logWarn() << "Coreginx not spawned. Triggering service start and autostart.";
        QFile::remove(coreginxDataDir + START_TRIGGER_FILE);
        touch(coreginxDataDir + START_TRIGGER_FILE);
    } else
        logInfo() << "Coreginx is already running";

    logInfo() << "Initializing Coreginx Helper..";
    logDebug() << "Lock name:" << cgxhPidFile;
    writeToFile(cgxhPidFile, QString::number(getpid()));

    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

    new SvdPublicWatcher();
    logInfo() << "Coreginx helper operational.";

    return app.exec();
}
