/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include <QtCore>

#include "../globals/globals.h"
#include "../service_spawner/utils.h"
#include "../notifications/notifications.h"
#include "../service_spawner/logger.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/service_watcher.h"
#include "../service_spawner/user_watcher.h"
#include "deployer.h"


void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch) {
    if (not QDir().exists(sourceRepositoryPath)) {
        logError() << "No source git repository found:" << sourceRepositoryPath;
        raise(SIGTERM);
    }
    auto clne = new SvdProcess("clone_repository", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName); // QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + serviceName;
    if (not QDir().exists(servicePath)) {
        logInfo() << "No Web Service dir found:" << servicePath << "Will be created";
        getOrCreateDir(servicePath);
    }

    QString linkFilePath = getOrCreateDir(servicePath + "/releases/");
    QFile *linkFile = new QFile(linkFilePath + "/current");
    QString command = QString("export DATE=\"app-$(date +%d%m%Y-%H%M%S)\"") +
        "&& cd " + servicePath + " > " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& git clone " + sourceRepositoryPath + " releases/${DATE}" + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& cd " + servicePath + "/releases/${DATE} " + " 2>&1 " +
        "&& git checkout -b " + branch + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " + /* branch might already exists */
        "; git pull origin " + branch + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "; cat " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE + " > " + servicePath + DEFAULT_SERVICE_PREVIOUS_RELEASE_FILE +
        "; printf \"${DATE}\" > " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE +
        "&& printf \"Repository update successful in release ${DATE}\" >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ";
    logDebug() << "COMMAND:" << command;

    clne->spawnProcess(command);
    clne->waitForFinished(-1);
    linkFile->close();
    linkFile->deleteLater();
    logInfo() << "Web app:" << serviceName << "cloned on branch:" << branch;
    clne->deleteLater();
}


void installDependencies(QString& serviceName, QString& domain) {
    /* setting up service domain */
    SvdProcess *clne = new SvdProcess("install_dependencies", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    logInfo() << "Installing service dependencies";
    clne->spawnProcess("cd " + servicePath + " && sofin dependencies >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage) {
    logInfo() << "Creating app environment";

    QString servicePath = getServiceDataDir(serviceName);
    QString domainFilePath = servicePath + DEFAULT_SERVICE_DOMAIN_FILE;
    logDebug() << "Writing domain:" << domain << "to file:" << domainFilePath;
    writeToFile(domainFilePath, domain);

    auto latestRelease = readFileContents(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE).trimmed();
    logDebug() << "Current release:" << latestRelease;
    auto appDetector = new WebAppTypeDetector(servicePath + "/releases/" + latestRelease);
    auto appType = appDetector->getType();
    auto typeName = appDetector->typeName;
    logDebug() << "Detected application type:" << typeName;
    delete appDetector;

    /* do app type specific action */
    QString envEntriesString = "";
    switch (appType) {
        case StaticSite: {} break;

        case UnicornRailsSite: {
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";
        } break;

        case RailsSite: {
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";
        } break;

        case NodeSite: {} break;
        case NoType: {} break;
    }

    /* write to service env file */
    QString envFilePath = servicePath + DEFAULT_SERVICE_ENV_FILE;
    writeToFile(envFilePath, envEntriesString);
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

    uint uid = getuid();

    /* web app name is simultanously a git repository name: */
    QString serviceName = "", stage = "staging", branch = "master", domain = "local.dev"; // appName.local.dev domain always points to 127.0.0.1, but will be almost valid TLD for services resolving domains.

    bool debug = true, trace = false;
    for (int i = 1; i < args.size(); ++i) {
        if (rxWebBranch.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                branch = args.at(i+1);
            }
        }
        if (rxWebDomain.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                domain = args.at(i+1);
            }
        }
        if (rxWebStage.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                stage = args.at(i+1);
            }
        }
        if (rxWebAppName.indexIn(args.at(i)) != -1 ) {
            if (i+1 < args.size()) {
                serviceName = args.at(i+1);
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

    if (serviceName.trimmed().isEmpty()) {
        logError() << "Name not given! Try 'svddeploy -n your-git-repo-name -o domain.name -b branch -s stage'";
        return EXIT_FAILURE;
    }

    logInfo() << "Deploying app: " << serviceName << "for stage:" << stage << "branch:" << branch << "at domain:" << domain;

    /* file lock setup */
    QString lockName = getHomeDir() + "/." + serviceName + ".deploying.pid";
    if (QFile::exists(lockName)) {
        bool ok;
        QString aPid = readFileContents(lockName).trimmed();
        uint pid = aPid.toInt(&ok, 10);
        if (ok) {
            if (pidIsAlive(pid) or pid == 0) { /* NOTE: if pid == 0 it means that SS is runned from SS root maintainer */
                logError() << "WAD is already running.";
                return LOCK_FILE_OCCUPIED_ERROR; /* can not open */
            } else
                logDebug() << "No alive WAD pid found";

        } else {
            logWarn() << "Pid file is damaged or doesn't contains valid pid. File will be removed";
            QFile::remove(lockName);
        }
    }
    logDebug() << "Lock name:" << lockName;
    writeToFile(lockName, QString::number(getpid()), false); /* get process pid and record it to pid file no logrotate */

    signal(SIGINT, unixSignalHandler);
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

    if (uid == 0) {
        logError() << "Web deployments as root are not allowed a.t.m.";
    }
    //     logInfo("Root Mode Service Spawner v" + QString(APP_VERSION) + ". " + QString(COPYRIGHT));
    //     setPublicDirPriviledges(getOrCreateDir(DEFAULT_PUBLIC_DIR));
    //     setupDefaultVPNNetwork();

    //     /* Setting up root watchers */
    //     new SvdUserWatcher();

    // } else {


    logInfo("Web App Deployer (WAD) v" + QString(APP_VERSION) + ". " + QString(COPYRIGHT));

    QString repositoryRootPath = QString(getenv("HOME")) + DEFAULT_GIT_REPOSITORY_DIR;
    getOrCreateDir(repositoryRootPath);
    QString repositoryPath = repositoryRootPath + serviceName + ".git";

    cloneRepository(repositoryPath, serviceName, branch);
    installDependencies(serviceName, domain);
    createEnvironmentFiles(serviceName, domain, stage);

    logInfo() << "Deploying app" << serviceName << "from repository:" << repositoryPath;
    //     logDebug() << "Checking user directory priviledges";
    //     setUserDirPriviledges(getHomeDir());

    //     /* Setting up user watchers */
    //     new SvdUserWatcher();
    // }
    logInfo() << "Deploy successful.";
    return EXIT_SUCCESS;
    // return app.exec();
}