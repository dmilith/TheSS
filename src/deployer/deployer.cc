/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "deploy.h"
#include "../death_watch/death_watch.h"


void deployerSignalHandler(int sigNum) {
    switch (sigNum) {
        case SIGINT: {
            logWarn() << "Caught Interrupt signal. Quitting application.";
            qApp->quit();
        } break;

        case SIGQUIT: {
            logInfo() << "Caught Quit signal. Quitting application.";
            qApp->quit();
        } break;

        case SIGABRT: {
            logWarn() << "Caught Abort signal! Execution aborted.";
            raise(SIGTERM);
        } break;

        default: {
            logWarn() << "Caught unhandled signal:" << strsignal(sigNum);
        }
    }
}


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

    /* web app name is simultanously a git repository name: */
    QString serviceName = "", stage = "staging", branch = "master", domain = QString(getenv("USER")) + ".dev"; // appName.env[USER].dev domain always points to 127.0.0.1, but will be almost valid TLD for services resolving domains.

    /* gathering errors and warnings */
    QStringList errors, warnings;

    bool debug = false, trace = false;
    for (int i = 1; i < args.size(); ++i) {
        if (rxWebBranch.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                branch = args.at(i+1);
            }
        }
        if (rxWebDomain.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                if (args.at(i+1).length() >= 4) { // NOTE: a.io - minimal FQDN length is 4, but also check domain allowed characters
                    QHostInfo info = QHostInfo::fromName(args.at(i+1)); /* XXX: NOTE: it's blocking and relies on DNS service available */
                    if (info.error() == QHostInfo::NoError)
                        domain = args.at(i+1);
                    else
                        errors << "Domain resolve failed for: " + args.at(i+1) + ". Cannot continue";

                } else
                    warnings << "Given domain name: " + args.at(i+1) + " seems to not be valid. Using standard one: " + domain;
            }
        }
        if (rxWebStage.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                stage = args.at(i+1);
            }
        }
        if (rxWebAppName.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                if (args.at(i+1).length() > 0)
                    serviceName = args.at(i+1);
                else
                    errors << "Your service name is empty. Cannot continue";

            }
        }
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
            cout << "ServeD Web App Deployer (WAD) v" << APP_VERSION << ". " << COPYRIGHT << endl;
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

    /* handle some POSIX signals */
    signal(SIGINT, deployerSignalHandler);
    signal(SIGABRT, deployerSignalHandler);
    signal(SIGQUIT, deployerSignalHandler);
    signal(SIGUSR1, deployerSignalHandler);
    signal(SIGUSR2, deployerSignalHandler);
    signal(SIGHUP, deployerSignalHandler);
    signal(SIGTTIN, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

    /* NOTE: make sure that web-app isn't already in deploying state for user */
    bool ok = false, failed = false;

    QString wadPidFile = getServiceDataDir(serviceName) + DEFAULT_SERVICE_DEPLOYING_FILE;
    QString aPid = readFileContents(wadPidFile).trimmed();
    uint pid = aPid.toInt(&ok, 10);
    if (ok) {
        if (not pidIsAlive(pid))
            failed = true;
        else {
            logError() << "Deploy aborted! WAD deploy state for service:" << serviceName << "is already in deploying state. You may need to manually resolve possible deployment issue.";
            raise(SIGTERM);
        }
    } else
        failed = true;

    if (failed) {
        logDebug() << "No alive pid of WAD for service:" << serviceName << "found. Resetting dead state for service deployment.";
        logDebug() << "Removing WAD deploying state file:" << wadPidFile;
        QFile::remove(wadPidFile);
    }

    /* setup new file lock with current pid */
    uint currentPid = getpid();
    logDebug() << "Setting up and writing pid:" << currentPid << "new lock name:" << wadPidFile;
    writeToFile(wadPidFile, QString::number(currentPid));

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

    if (serviceName.trimmed().isEmpty()) {
        logError() << "Name not given! Try 'svddeploy -n your-git-repo-name -o domain.name -b branch -s stage'";
        return EXIT_FAILURE;
    }

    logInfo() << "ServeD Web-App-Deployer (WAD)" << QString("v") + APP_VERSION << COPYRIGHT;
    logInfo() << "Deploy started for service:" << serviceName << "(stage:" << stage << "from branch:" << branch << "destination domain:" << domain << ")";

    if (getuid() == 0) {
        logError() << "Web deployments as root are not allowed a.t.m. for security reasons";
        raise(SIGTERM);
    }

    auto diskMap = getDiskFree(getenv("HOME"));
    Q_FOREACH(auto map, diskMap.keys()) {
        auto value = diskMap.take(map);
        if (value < MINIMUM_DISK_SPACE_IN_MEGS) {
            logError() << "Insufficient disk space (less than " << QString::number(MINIMUM_DISK_SPACE_IN_MEGS) << "MiB) detected on remote destination machine. Deploy aborted!";
            QFile::remove(wadPidFile);
            raise(SIGTERM);

        } else
            logInfo() << "Sufficient disk space detected for:" << map << "(" << QString::number(value) << " of required" << MINIMUM_DISK_SPACE_IN_MEGS << "MiB)";
    }

    createEnvironmentFiles(serviceName, domain, stage, branch);

    logInfo() << "Deploy successful. Cleaning deploying state.";
    QFile::remove(wadPidFile);
    removeDir(getServiceDataDir(serviceName) + DEFAULT_RELEASES_DIR + "build-in-progress-" + stage, true);

    return EXIT_SUCCESS;
}
