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

    /* create "deploying" state */
    touch(servicePath + DEFAULT_SERVICE_DEPLOYING_FILE);
    logDebug() << "Created deploying state in file:" << servicePath + DEFAULT_SERVICE_DEPLOYING_FILE << " for service:" << serviceName;

    getOrCreateDir(servicePath + "/releases/");
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
    logInfo() << "Web app:" << serviceName << "cloned on branch:" << branch;
    clne->deleteLater();
}


void installDependencies(QString& serviceName) {
    /* setting up service domain */
    SvdProcess *clne = new SvdProcess("install_dependencies", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    logInfo() << "Installing service dependencies";
    auto latestRelease = readFileContents(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE).trimmed();
    auto latestReleaseDir = servicePath + "/releases/" + latestRelease;

    clne->spawnProcess("cd " + latestReleaseDir + " && sofin dependencies >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain) {
    QStringList forbiddenSpawnDeps; /* dependencies forbidden to spawn */
    forbiddenSpawnDeps << "ruby" << "node" << "python" << "python-legacy" << "imagemagick" << "graphicsmagick"; // XXX: hardcoded

    QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
    QString deps = readFileContents(depsFile).trimmed();

    /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
    QStringList appDependencies = deps.split("\n");
    logDebug() << "Gathered dependencies:" << appDependencies << "of size:" << appDependencies.size();
    QString jsonResult = "\"dependencies\": [";

    /* filter forbiddens */
    for (int i = 0; i < appDependencies.size(); i++) {
        QString d1 = appDependencies.at(i);
        if (forbiddenSpawnDeps.contains(d1))
            appDependencies[i] = "";
    }
    appDependencies.removeAll("");

    if (appDependencies.size() == 0) {
        logInfo() << "Empty list of dependencies software, that acts, like some kind of a server.";
        return jsonResult + "], "; /* return empty list */
    }

    for (int indx = 0; indx < appDependencies.size() - 1; indx++) {
        QString dep = "\"" + appDependencies.at(indx);
        dep[0] = dep.at(0).toUpper();
        jsonResult += dep + "\", ";
    }

    QString last = "\"" + appDependencies.at(appDependencies.size() - 1);
    last[0] = last.at(0).toUpper();
    jsonResult += last + "\"], ";
    jsonResult += QString(" \"configure\": {\"commands\": \"") + "svddeployer -n " + serviceName + " -b " + branch + " -o " + domain + "\"},";
    return jsonResult;
}


void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch) {

    logInfo() << "Creating app environment";
    QString servicePath = getServiceDataDir(serviceName);
    QString domainFilePath = servicePath + DEFAULT_SERVICE_DOMAIN_FILE;
    logDebug() << "Writing domain:" << domain << "to file:" << domainFilePath;
    writeToFile(domainFilePath, domain);

    auto latestRelease = readFileContents(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE).trimmed();
    logDebug() << "Current release:" << latestRelease;
    auto latestReleaseDir = servicePath + "/releases/" + latestRelease;
    logDebug() << "Release path:" << latestReleaseDir;
    auto appDetector = new WebAppTypeDetector(latestReleaseDir);
    auto appType = appDetector->getType();
    QString envEntriesString = "";
    logDebug() << "Detected application type:" << appDetector->typeName;
    delete appDetector;

    /* do app type specific action */
    SvdProcess *clne = new SvdProcess("create_environment", getuid(), false);

    switch (appType) {

        case StaticSite: {

            QString jsonResult = "{\"alwaysOn\": false, \"watchPort\": false, ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);

            jsonResult += QString("\"start\": {\"commands\": \"echo 'Static app ready' >> SERVICE_PREFIX") + DEFAULT_SERVICE_LOG_FILE + " 2>&1 &" + "\"} }";
            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            logInfo() << "Generating http proxy configuration";
            QString port = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + "/" + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
            QString contents = nginxEntry(appType, latestReleaseDir);

            logDebug() << "Generated proxy contents:" << contents;
            writeToFile(servicePath + DEFAULT_PROXY_FILE, contents);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Re-Launching service using newly generated igniter.";
            touch(servicePath + RESTART_TRIGGER_FILE);

        } break;


        case RubySite: {
            logInfo() << "Preparing service to start";
            getOrCreateDir(latestReleaseDir + "/../../shared/" + stage + "/public/shared"); /* /public usually exists */
            getOrCreateDir(latestReleaseDir + "/../../shared/" + stage + "/log");
            getOrCreateDir(latestReleaseDir + "/../../shared/" + stage + "/tmp");
            getOrCreateDir(latestReleaseDir + "/../../shared/" + stage + "/config");
            getOrCreateDir(latestReleaseDir + "/public");
            logInfo() << "Purging app release dir";
            removeDir(latestReleaseDir + "/log");
            removeDir(latestReleaseDir + "/tmp");

            logInfo() << "Symlinking and copying shared directory in current release";
            clne->spawnProcess("cd " + latestReleaseDir + " && ln -sv ../../../shared/" + stage + "/public/shared public/shared >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);
            clne->spawnProcess("cd " + latestReleaseDir + " &&\n\
                cd ../../shared/" + stage + "/config/ \n\
                for i in *; do \n\
                    cp -v $(pwd)/$i " + latestReleaseDir + "/config/$i >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 \n\
                done \n\
            ");
            clne->waitForFinished(-1);
            clne->spawnProcess(" cd " + latestReleaseDir + " && ln -sv ../../shared/" + stage + "/log log >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);
            clne->spawnProcess("cd " + latestReleaseDir + " && ln -sv ../../shared/" + stage + "/tmp tmp >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            /* generate database.yml for Ruby app */
            QString databaseName = serviceName + "-" + stage;
            QString database;
            QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
            QString deps = "";

            if (QFile::exists(depsFile)) { /* NOTE: special software list file from Sofin */
                deps = readFileContents(depsFile).trimmed();

                if (deps.trimmed().toLower().contains("postgres")) { /* postgresql specific configuration */
                    logInfo() << "Detected Postgresql dependency in file:" << depsFile;
                    database = "Postgresql";
                    QString content = stage + ": \n\
  adapter: postgresql \n\
  encoding: unicode \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  pool: 5 \n\
  port: <%= File.read(ENV['HOME'] + \"/SoftwareData/Postgresql/.ports/0\") %> \n\
  host: <%= ENV['HOME'] + \"/SoftwareData/Postgresql/\" %> \n\
"; // XXX: should contains latestRelease cause of potential database failure that might happen after db:migrate
                    writeToFile(servicePath + "/shared/" + stage + "/config/database.yml", content);
                }
                if (deps.trimmed().toLower().contains("mysql")) {
                    logInfo() << "Detected Mysql dependency in file:" << depsFile;
                    database = "Mysql";
                    writeToFile(depsFile, stage + ":"); // XXX: unfinished
                }
            }

            /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
            QStringList appDependencies = deps.split("\n");
            logDebug() << "Gathering dependencies:" << appDependencies;
            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);

            jsonResult += QString(" \"start\": {\"commands\": \"") + "cd " + latestReleaseDir + " && RAKE_ENV=" + stage + " RAILS_ENV=" + stage + " SSL_CERT_FILE=SERVICE_PREFIX" + DEFAULT_SSL_CA_FILE + " bundle exec rails s -b " + DEFAULT_LOCAL_ADDRESS + " -p $(sofin port " + serviceName + ") -P SERVICE_PREFIX" + DEFAULT_SERVICE_PID_FILE + " >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 &" + "\"} }";

            logDebug() << "Generated Igniter JSON:" << jsonResult;

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);
            /* end of igniter creation code */

            QString cacertLocation = QString(DEFAULT_CA_CERT_ROOT_SITE) + DEFAULT_SSL_CA_FILE;
            logInfo() << "Gathering SSL CA certs from:" << cacertLocation << "if necessary.";
            clne->spawnProcess("cd " + servicePath + " && test ! -f " + DEFAULT_SSL_CA_FILE + " && curl -C - -L -O " + cacertLocation + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1");
            clne->waitForFinished(-1);

            logInfo() << "Installing bundle for stage:" << stage << "of Rails Site";
            getOrCreateDir(servicePath + "/bundle-" + stage);
            clne->spawnProcess("cd " + latestReleaseDir + " && RAKE_ENV=" + stage + " RAILS_ENV=" + stage + " SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + " bundle install --path " + servicePath + "/bundle-" + stage + " --without test development >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            logInfo() << "Building assets";
            clne->spawnProcess("cd " + latestReleaseDir + " && RAKE_ENV=" + stage + " RAILS_ENV=" + stage + " SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + " rake assets:precompile >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            if (database != "") { /* empty database means no database dependencies in web app */
                logInfo() << "Launching database service:" << database;
                touch(QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + database + START_TRIGGER_FILE);
            }
            logInfo() << "Running database setup";
            logDebug() << "Creating user:" << databaseName;
            clne->spawnProcess("createuser -s -d -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/Postgresql -p $(sofin port Postgresql) " + databaseName + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);
            logDebug() << "Creating database:" << databaseName;
            clne->spawnProcess("createdb -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/Postgresql -p $(sofin port Postgresql) -O " + databaseName + " " + databaseName + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            logInfo() << "Running database migrations";
            clne->spawnProcess("cd " + latestReleaseDir + " && RAKE_ENV=" + stage + " RAILS_ENV=" + stage + " SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + " rake db:migrate db:seed >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            logInfo() << "Generating http proxy configuration";
            QString port = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + "/" + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
            QString contents = nginxEntry(appType, latestReleaseDir, domain, serviceName, stage, port);
            logDebug() << "Generated proxy contents:" << contents;
            writeToFile(servicePath + DEFAULT_PROXY_FILE, contents);

            logDebug() << "Appending ENV settings";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";

            logInfo() << "Re-Launching service using newly generated igniter.";
            touch(servicePath + RESTART_TRIGGER_FILE);


        } break;


        case NodeSite: {
            logInfo() << "Installing npm modules for Nodejs Site";
            clne->spawnProcess("cd " + latestReleaseDir + " && npm install >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " + " && touch " + servicePath + "/" + DEFAULT_SERVICE_CONFIGURED_FILE);
            clne->waitForFinished(-1);

        } break;


        case NoType: {
            logError() << "No web application detected in service directory:" << servicePath;
            raise(SIGTERM);

        } break;
    }

    logInfo() << "Invoking bin/build of project (if exists)";
    clne->spawnProcess("cd " + latestReleaseDir + " && test -x bin/build && bin/build >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);

    /* write to service env file */
    if (not envEntriesString.isEmpty()) {
        QString envFilePath = servicePath + DEFAULT_SERVICE_ENV_FILE;
        writeToFile(envFilePath, envEntriesString);
    }

    clne->deleteLater();
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

    logInfo() << "Deploying app:" << serviceName << "for stage:" << stage << "branch:" << branch << "at domain:" << domain;

    /* file lock setup */
    QString lockName = getServiceDataDir(serviceName) + DEFAULT_SERVICE_DEPLOYING_FILE;
    if (QFile::exists(lockName)) {
        logError() << "WAD is already running.";
        return LOCK_FILE_OCCUPIED_ERROR; /* can not open */
    }
    logDebug() << "Lock name:" << lockName;
    writeToFile(lockName, QString::number(getpid()));

    signal(SIGINT, unixSignalHandler);
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

    if (getuid() == 0) {
        logError() << "Web deployments as root are not allowed a.t.m.";
        raise(SIGTERM);
    }

    logInfo("Web App Deployer (WAD) v" + QString(APP_VERSION) + ". " + QString(COPYRIGHT));

    QString repositoryRootPath = QString(getenv("HOME")) + DEFAULT_GIT_REPOSITORY_DIR;
    getOrCreateDir(repositoryRootPath);
    QString repositoryPath = repositoryRootPath + serviceName + ".git";

    cloneRepository(repositoryPath, serviceName, branch);
    installDependencies(serviceName);
    createEnvironmentFiles(serviceName, domain, stage, branch);

    logInfo() << "Deploy successful. Cleaning deploying state and locks";
    QFile::remove(getServiceDataDir(serviceName) + DEFAULT_SERVICE_DEPLOYING_FILE);
    QFile::remove(lockName);
    return EXIT_SUCCESS;
}