/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "../deployer/deploy.h"
#include "public_watcher.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    QStringList args = app.arguments();
    bool background = false; /* by default launch svddeployer as foreground task */
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

    /* NOTE: make sure TheSS is running for user, and launch it if it's not: */
    bool ok = false;
    QString cgxhPidFile = getHomeDir() + "/.coreginx_helper.pid";
    QString aPid = readFileContents(cgxhPidFile).trimmed();
    uint pid = aPid.toInt(&ok, 10);
    if (ok) {
        if (not pidIsAlive(pid)) {
        } else {
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
    if (getuid() != 0) {
        logError() << "Root account is necessary for Coreginx Helper.";
        raise(SIGTERM);
    }


    logInfo() << "Initializing Coreginx Helper..";

    logDebug() << "Lock name:" << cgxhPidFile;
    writeToFile(cgxhPidFile, QString::number(getpid()));

    signal(SIGINT, unixSignalHandler);
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

    new SvdPublicWatcher();
    logInfo() << "Coreginx helper operational.";

    return app.exec();
}
