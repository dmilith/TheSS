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


void generateDatastoreSetup(WebDatastore db, QString serviceName, QString stage, WebAppTypes appType) {
    QString databaseName = serviceName + "-" + stage;
    QString servicePath = getServiceDataDir(serviceName);

    switch (db) {

        case Postgresql: {
            switch (appType) {
                case RubySite:
                    writeToFile(servicePath + "/shared/" + stage + "/config/database.yml",
stage + ": \n\
  adapter: postgresql \n\
  encoding: unicode \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  pool: 5 \n\
  port: <%= File.read(ENV['HOME'] + \"/SoftwareData/" + getDbName(db) + "/.ports/0\") %> \n\
  host: <%= ENV['HOME'] + \"/SoftwareData/" + getDbName(db) + "/\" %> \n");
                    break;

                default: break;
            }
        } break;

        case Mysql: {
            // return ""; // NOTE: NYI
        } break;

        case Mongo: {
            // return ""; // NOTE: NYI
        } break;

        case Redis: {
            // return ""; // NOTE: NYI
        } break;

        case ElasticSearch: {
            // return ""; // NOTE: NYI
        } break;

        case Sphinx: {
            // return ""; // NOTE: NYI
        } break;

        case NoDB: {
            switch (appType) {
                case RubySite:
                    writeToFile(servicePath + "/shared/" + stage + "/config/database.yml",
stage + ": \n\
  adapter: sqlite3 \n\
  database: db/db_" + databaseName + "_" + stage + ".sqlite3 \n\
  timeout: 5000 \n");
                break;

                default: break;
            }

        } /* NoDB means we might want to use SQLite3 driver */
    }
}


void generateServicePorts(QString servicePath, int amount) {
    /* generate default port for service */
    if (amount > 100) {
        logWarn() << "Are you serious? You want to reserve more than a hundred ports?";
    }
    if (amount < 1) {
        amount = 1;
    }
    QString portsDir = servicePath + QString(DEFAULT_SERVICE_PORTS_DIR);
    getOrCreateDir(portsDir);
    QString portFilePath = portsDir + QString(DEFAULT_SERVICE_PORT_NUMBER); /* default port */
    if (not QFile::exists(portFilePath)) {
        int port = registerFreeTcpPort(abs((rand() + 1024) % 65535));
        logDebug() << "Generated main port:" << QString::number(port);
        writeToFile(portFilePath, QString::number(port));
    }
    for (int i = 2; i < amount + 1; i++) {
        QString backupPortFilePath = portsDir + QString::number(i - 1);
        if (not QFile::exists(backupPortFilePath)) {
            int port = registerFreeTcpPort(abs((rand() + 1024) % 65535));
            logDebug() << "Generated additional port:" << QString::number(port);
            writeToFile(backupPortFilePath, QString::number(port));
        }
    }
}


bool validateNginxEntry(QString& servicePath, QString contents) {
    QString prefix = "events { worker_connections 1024; } http { error_log off; access_log off; ";
    QString postfix = " }";
    QString uuid = QUuid::createUuid().toString();
    QString uuidFile = servicePath + "/" + uuid;
    QString testFile = "/tmp/proxy.conf-" + uuid;

    QString genContents = contents.replace("listen 80", "listen " + QString::number(registerFreeTcpPort())); /* replace defaul port 80 with some bogus port */
    writeToFile(testFile, prefix + genContents + postfix);

    logDebug() << "Generated contents will be validated:" << prefix + genContents + postfix;
    logDebug() << "Validation confirmation UUID:" << uuid << "in file:" << uuidFile;

    getOrCreateDir("/tmp/logs");
    auto clne = new SvdProcess("nginx_entry_validate", getuid(), false);
    clne->spawnProcess("nginx -t -c " + testFile + " -p /tmp && touch " + uuidFile);
    clne->waitForFinished(DEFAULT_SERVICE_PAUSE_INTERVAL); /* give it some time */
    clne->deleteLater();

    if (QFile::exists(uuidFile)) {
        logInfo() << "Nginx entry validation passed.";
        logDebug() << "Removing confirmation file:" << uuidFile;
        QFile::remove(uuidFile);
        QFile::remove(testFile);
        return true;
    }
    QFile::remove(testFile);
    return false; /* means failure by definition */
}


void prepareHttpProxy(QString& servicePath, WebAppTypes appType, QString& latestReleaseDir, QString& domain, QString& serviceName, QString& stage) {
    logInfo() << "Generating http proxy configuration";
    QString port = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
    QString contents = nginxEntry(appType, latestReleaseDir, domain, serviceName, stage, port);
    if (validateNginxEntry(servicePath, contents)) {
        logDebug() << "Generated proxy contents:" << contents;
        writeToFile(servicePath + DEFAULT_PROXY_FILE, contents);
    } else {
        logWarn() << "Web-App Validation failed, cause of a failure in generated nginx proxy file. Proxy file generation skipped!";
    }
}


void prepareSharedDirs(QString& latestReleaseDir, QString& servicePath, QString& stage) {
    logInfo() << "Preparing shared dir for service start";
    getOrCreateDir(servicePath + "/shared/" + stage + "/public/shared"); /* /public usually exists */
    getOrCreateDir(servicePath + "/shared/" + stage + "/log");
    getOrCreateDir(servicePath + "/shared/" + stage + "/tmp");
    getOrCreateDir(servicePath + "/shared/" + stage + "/config");
    getOrCreateDir(latestReleaseDir + "/public");
    logInfo() << "Purging app release /log and /tmp dirs.";
    removeDir(latestReleaseDir + "/log");
    removeDir(latestReleaseDir + "/tmp");
}


void prepareSharedSymlinks(QString& latestReleaseDir, QString& servicePath, QString& stage) {
    auto clne = new SvdProcess("shared_symlinks", getuid(), false);
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
    clne->deleteLater();
}


void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch) {
    if (not QDir().exists(sourceRepositoryPath)) {
        logError() << "No source git repository found:" << sourceRepositoryPath;
        raise(SIGTERM);
    }
    auto clne = new SvdProcess("clone_repository", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    if (not QDir().exists(servicePath)) {
        logInfo() << "No Web Service dir found:" << servicePath << "Will be created";
        getOrCreateDir(servicePath);
    }

    /* create "deploying" state */
    touch(servicePath + DEFAULT_SERVICE_DEPLOYING_FILE);
    logDebug() << "Created deploying state in file:" << servicePath + DEFAULT_SERVICE_DEPLOYING_FILE << "for service:" << serviceName;

    getOrCreateDir(servicePath + "/releases/");

    logInfo() << "Cleaning old deploys - over count of:" << QString::number(MAX_DEPLOYS_TO_KEEP);
    QStringList gatheredReleases = QDir(servicePath + "/releases/").entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    QStringList releases;
    if (gatheredReleases.size() > MAX_DEPLOYS_TO_KEEP) {
        for (int i = 0; i < MAX_DEPLOYS_TO_KEEP; i++) {
            releases << gatheredReleases.at(i);
        }
        logDebug() << "Releases left:" << releases;
        Q_FOREACH(QString release, gatheredReleases) {
            if (not releases.contains(release)) {
                logDebug() << "Removing old release:" << servicePath + "/releases/" + release;
                clne->spawnProcess("rm -rf " + servicePath + "/releases/" + release);
                clne->waitForFinished(-1);
            }
        }
    }

    QString command = QString("export DATE=\"app-$(date +%d%m%Y-%H%M%S)\"") +
        "&& cd " + servicePath + " > " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& sofin reload > " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& git clone " + sourceRepositoryPath + " releases/${DATE}" + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& cd " + servicePath + "/releases/${DATE} " + " 2>&1 " +
        "&& git checkout -b " + branch + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " + /* branch might already exists */
        "; git pull origin " + branch + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "; cat " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE + " > " + servicePath + DEFAULT_SERVICE_PREVIOUS_RELEASE_FILE +
        "; cat " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE + " >> " + servicePath + DEFAULT_SERVICE_RELEASES_HISTORY +
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
    auto clne = new SvdProcess("install_dependencies", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    logInfo() << "Installing service dependencies";
    auto latestRelease = readFileContents(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE).trimmed();
    auto latestReleaseDir = servicePath + "/releases/" + latestRelease;

    clne->spawnProcess("cd " + latestReleaseDir + " && sofin dependencies >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


void spawnBinBuild(QString& latestReleaseDir, QString& serviceName, QString& servicePath, QStringList appDependencies) {
    auto clne = new SvdProcess("spawn_bin_build", getuid(), false);
    logInfo() << "Invoking bin/build of project (if exists)";
    clne->spawnProcess("cd " + latestReleaseDir + " && test -x bin/build && " + buildEnv(serviceName, appDependencies) + " bin/build >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain) {
    QStringList allowedToSpawnDeps; /* dependencies allowed to spawn as independenc service */
    allowedToSpawnDeps << "postgresql" << "mysql" << "redis" << "redis-usock" << "nginx" << "passenger" << "sphinx" << "memcached" << "elasticsearch"; // XXX: hardcoded
    // TODO: define Sphinx igniter

    QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
    QString deps = readFileContents(depsFile).trimmed();

    /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
    QStringList appDependencies = deps.split("\n");
    logDebug() << "Gathered dependencies:" << appDependencies << "of size:" << appDependencies.size();
    QString jsonResult = "\"dependencies\": [";

    /* filter forbiddens */
    for (int i = 0; i < appDependencies.size(); i++) {
        QString d1 = appDependencies.at(i);
        if (not allowedToSpawnDeps.contains(d1))
            appDependencies[i] = "";
    }
    appDependencies.removeAll("");

    if (appDependencies.size() == 0) {
        logInfo() << "Empty list of dependencies software, that acts, like some kind of a server.";
        return jsonResult + "], "; /* return empty list */
    }


    Q_FOREACH(auto val, appDependencies) {
        val[0] = val.at(0).toUpper();
        QString location = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + val;
        getOrCreateDir(location);
        QFile::remove(location + START_TRIGGER_FILE);
        touch(location + START_TRIGGER_FILE);

        int steps = 0;
        while (not QFile::exists(location + DEFAULT_SERVICE_RUNNING_FILE)) {
            logDebug() << "Still waiting for service:" << val;
            sleep(1);
            steps++;
            if (steps > OLD_SERVICE_SHUTDOWN_TIMEOUT) {
                logError() << "Exitting endless loop, cause service:" << val << "refuses to go down after " << QString::number(steps -1) << " seconds!";
                break;
            }
        }
    }

    for (int indx = 0; indx < appDependencies.size() - 1; indx++) {
        QString elm = appDependencies.at(indx);
        elm[0] = elm.at(0).toUpper();
        logInfo() << "Spawning depdendency:" << elm;
        QString location = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + elm;
        getOrCreateDir(location);
        QFile::remove(location + START_TRIGGER_FILE);
        touch(location + START_TRIGGER_FILE);
        jsonResult += "\"" + elm + "\", ";
    }

    QString elmLast = appDependencies.at(appDependencies.size() - 1);
    elmLast[0] = elmLast.at(0).toUpper();
    jsonResult += "\"" + elmLast + "\"], ";
    jsonResult += QString("\n\n\"configure\": {\"commands\": \"") + "svddeployer -n " + serviceName + " -b " + branch + " -o " + domain + "\"},";

    logInfo() << "Spawning depdendency:" << elmLast;
    QString location = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + elmLast;
    getOrCreateDir(location);
    QFile::remove(location + START_TRIGGER_FILE);
    touch(location + START_TRIGGER_FILE);
    logDebug() << "DEBUG: jsonResult:" << jsonResult;
    return jsonResult;
}


QString buildEnv(QString& serviceName, QStringList deps) {
    QString serviceEnvFile = getServiceDataDir(serviceName) + DEFAULT_SERVICE_ENV_FILE;
    QString result = " ";

    Q_FOREACH(QString fragment, deps) {
        QString serviceDepsFile = getServiceDataDir(fragment) + DEFAULT_SERVICE_ENV_FILE;
        if (QFile::exists(serviceDepsFile)) {
            QStringList innerContents = readFileContents(serviceDepsFile).trimmed().split('\n');
            logDebug() << "innerCont:" << innerContents;
            Q_FOREACH(QString part, innerContents) {
                result += part + " ";
            }
        }
    }

    if (QFile::exists(serviceEnvFile)) {
        QStringList contents = readFileContents(serviceEnvFile).trimmed().split('\n');
        Q_FOREACH(QString part, contents) {
            result += part + " ";
        }
        logDebug() << "Built env string:" << result;
    }
    return result;
}


WebDatastore detectDatastore(QString& deps, QString& depsFile) {
    if (deps.trimmed().toLower().contains("postgres")) { /* postgresql specific configuration */
        logInfo() << "Detected Postgresql dependency in file:" << depsFile;
        return Postgresql;
    }
    if (deps.trimmed().toLower().contains("mysql")) {
        logInfo() << "Detected Mysql dependency in file:" << depsFile;
        return Mysql;
    }

    logWarn() << "Falling back to SqLite3 driver cause no database defined in dependencies";
    return NoDB;
}


QString getDbName(WebDatastore db) {
    switch (db) {
        case Postgresql: return "Postgresql";
        case Mysql: return "Mysql";
        case Mongo: return "Mongo";
        case Redis: return "Redis";
        case ElasticSearch: return "ElasticSearch";
        case Sphinx: return "Sphinx";
        case NoDB: return "NoDB";
    }
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
    auto clne = new SvdProcess("create_environment", getuid(), false);
    QStringList appDependencies;

    switch (appType) {

        case StaticSite: {

            QString jsonResult = "{\"alwaysOn\": false, \"watchPort\": false, ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            jsonResult += QString("\n\n\"start\": {\"commands\": \"echo 'Static app ready' >> SERVICE_PREFIX") + DEFAULT_SERVICE_LOG_FILE + " 2>&1 &" + "\"}\n}";
            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);
            logInfo() << "Launching service:" << serviceName;

            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case RubySite: {

            QString databaseName = serviceName + "-" + stage;
            WebDatastore database = NoDB;
            QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
            QString envFilePath = servicePath + DEFAULT_SERVICE_ENV_FILE;
            QString deps = "";

            if (QFile::exists(depsFile)) { /* NOTE: special software list file from Sofin, called ".dependencies" */
                deps = readFileContents(depsFile).trimmed();
            }
            database = detectDatastore(deps, depsFile);
            prepareSharedDirs(latestReleaseDir, servicePath, stage);
            generateDatastoreSetup(database, serviceName, stage, appType);

            /* write to service env file */
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";
            writeToFile(envFilePath, envEntriesString);

            generateServicePorts(servicePath);

            /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
            appDependencies = deps.split("\n");
            logDebug() << "Gathering dependencies:" << appDependencies;
            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, ";
            QString environment = buildEnv(serviceName, appDependencies);
            logDebug() << "Generateed Service Environment:" << environment;
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            QString startResultJson = ""; /* command that actually launches main app */

            QMap<QString, QString> serviceWorkers; /* additional workers of service: (startCommands, stopCommands) */
            QString procFile = latestReleaseDir + "/Procfile"; /* heroku compatible procfile */
            if (QFile::exists(procFile)) {
                QStringList entries = readFileContents(procFile).trimmed().split("\n");
                logInfo() << "Proceeding with Procfile entries:" << entries;

                Q_FOREACH(QString entry, entries) {
                    QString procfileHead = entry.split(":").at(0);
                    QString procfileTail = entry.split(":").at(1);
                    QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
                    procfileTail = procfileTail.replace("$PORT", servPort); /* replace $PORT value of Procfile if exists */

                    if (procfileHead == "web") { /* web worker is defined here */
                        logInfo() << "Found web worker:" << procfileHead;
                        logDebug() << "Worker entry:" << procfileTail << "on port:" << servPort;
                        startResultJson += " cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " bundle exec " + procfileTail + " -b " + DEFAULT_LOCAL_ADDRESS + " -p " + servPort + " -P SERVICE_PREFIX" + DEFAULT_SERVICE_PID_FILE + " >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 & ";
                    } else {
                        logInfo() << "Found an entry:" << procfileHead;
                        QString procPidFile = procfileHead + ".pid";

                        serviceWorkers.insert( /* NOTE: by default, each worker must accept pid location, log location and daemon mode */

                            /* (start commands, stop commands) : */
                            "cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " bundle exec " + procfileTail + " -P " + servicePath + "/" + procPidFile + " -L " + servicePath + DEFAULT_SERVICE_LOG_FILE + "-" + procfileHead + " -d && \n echo 'Started worker " + procfileHead + "' >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ",

                            /* , stop commands) : */
                            "svddw $(cat " + servicePath + "/" + procPidFile + ") >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 "

                        );
                    }
                }

                /* generate correct order of application execution after workers */
                jsonResult += QString("\n\n\"start\": {\"commands\": \"");
                Q_FOREACH(QString part, serviceWorkers.keys()) { /* keys => start commands */
                    jsonResult += part + " &&\n";
                }
                jsonResult += startResultJson;
                jsonResult += "\"}";

                Q_FOREACH(QString acmd, serviceWorkers.keys()) {
                    QString cmd = serviceWorkers.take(acmd);
                    jsonResult += QString(", \n\n\"stop\": {\"commands\": \"");
                    jsonResult += cmd + " ";
                }
                jsonResult += "\"}\n}";

            } else { /* generate standard igniter entry */

                logInfo() << "Generating default entry (no Procfile used)";
                jsonResult += QString("\n\n\"start\": {\"commands\": \"") + "cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " bundle exec rails s -b " + DEFAULT_LOCAL_ADDRESS + " -p $(sofin port " + serviceName + ") -P SERVICE_PREFIX" + DEFAULT_SERVICE_PID_FILE + " >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 &" + "\"}\n}";
            }
            logDebug() << "Generated Igniter JSON:" << jsonResult;

            int steps = 0;
            while (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE)) {
                logDebug() << "Older service already running. Invoking stop for:" << serviceName;
                touch(servicePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);
                sleep(1);
                steps++;
                if (steps > OLD_SERVICE_SHUTDOWN_TIMEOUT) {
                    logError() << "Exitting endless loop, cause service:" << serviceName << "refuses to go down after " << QString::number(steps -1) << " seconds!";
                    break;
                }
            }

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            QString cacertLocation = QString(DEFAULT_CA_CERT_ROOT_SITE) + DEFAULT_SSL_CA_FILE;
            logInfo() << "Gathering SSL CA certs from:" << cacertLocation << "if necessary.";
            clne->spawnProcess("cd " + servicePath + " && test ! -f " + DEFAULT_SSL_CA_FILE + " && curl -C - -L -O " + cacertLocation + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1");
            clne->waitForFinished(-1);

            logInfo() << "Installing bundle for stage:" << stage << "of Rails Site";
            getOrCreateDir(servicePath + "/bundle-" + stage);
            clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle install --path " + servicePath + "/bundle-" + stage + " --without test development >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            prepareSharedSymlinks(latestReleaseDir, servicePath, stage);

            logInfo() << "Building assets";
            clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle exec rake assets:precompile >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            logInfo() << "Running database setup for database:" << getDbName(database);
            switch (database) {
                case Postgresql: {
                    logDebug() << "Creating user:" << databaseName;
                    clne->spawnProcess("createuser -s -d -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + getDbName(database) + " -p $(sofin port " + getDbName(database) + ") " + databaseName + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
                    clne->waitForFinished(-1);
                    logDebug() << "Creating database:" << databaseName;
                    clne->spawnProcess("createdb -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + getDbName(database) + " -p $(sofin port " + getDbName(database) + ") -O " + databaseName + " " + databaseName + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
                    clne->waitForFinished(-1);

                } break;

                default: break;

            }

            logInfo() << "Running database migrations";
            if (database != Postgresql) { /* postgresql db creation is already done before this hook */
                clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle exec rake db:create >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
                clne->waitForFinished(-1);
            }
            clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle exec rake db:migrate >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Relaunching service using newly generated igniter.";
            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case NodeSite: {

            prepareSharedDirs(latestReleaseDir, servicePath, stage);
            prepareSharedSymlinks(latestReleaseDir, servicePath, stage);

            generateServicePorts(servicePath, 2); /* XXX: 2 ports for node by default */

            /* generate env and write it to service.env file */
            QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
            QString websocketsPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + "/1").trimmed(); // XXX: hardcoded
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "NODE_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "NODE_ENV=" + stage + "\n";
            envEntriesString += "NODE_PORT=" + servPort + "\n";
            envEntriesString += "NODE_DOMAIN=" + domain + "\n";
            envEntriesString += "NODE_WEBSOCKET_PORT=" + websocketsPort + "\n";
            envEntriesString += "NODE_WEBSOCKET_CHANNEL_NAME=" + serviceName + "-" + domain + "\n";
            QString envFilePath = servicePath + DEFAULT_SERVICE_ENV_FILE;
            writeToFile(envFilePath, envEntriesString);

            QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
            QString deps = "", content = "";

            /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
            if (QFile::exists(depsFile)) { /* NOTE: special software list file from Sofin */
                deps = readFileContents(depsFile).trimmed();
            }
            appDependencies = deps.split("\n");
            logDebug() << "Gathering dependencies:" << appDependencies;
            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"portsPool\": 2, ";
            QString environment = buildEnv(serviceName, appDependencies);
            logDebug() << "Generateed Service Environment:" << environment;
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            jsonResult += QString("\n\n\"start\": {\"commands\": \"") + "cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + "bin/app 2>&1 &" + "\"}\n}"; /* bin/app has to get all settings from ENV (stage in NODE_ENV) */
            logDebug() << "Generated Igniter JSON:" << jsonResult;

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE)) {
                logInfo() << "Older service already running. Invoking stop for:" << serviceName;
                touch(servicePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);
            }

            logInfo() << "Installing npm modules for stage:" << stage << "of Node Site";
            clne->spawnProcess("cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " npm install >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Relaunching service using newly generated igniter.";
            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case PhpSite: {

            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"softwareName\": \"Php\", ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            #ifdef __APPLE__
                logError() << "Apple PHP deployments aren't supported yet!";
                raise(SIGTERM);
            #endif

            generateServicePorts(servicePath);

            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE)) {
                logInfo() << "Older service already running. Invoking stop for:" << serviceName;
                touch(servicePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);
            }

            jsonResult += QString("\n\n\"start\": {\"commands\": \"" + buildEnv(serviceName, appDependencies) + " SERVICE_ROOT/exports/php-fpm -c SERVICE_PREFIX/service.ini --fpm-config SERVICE_PREFIX/service.conf --pid SERVICE_PREFIX/service.pid -D && \n echo 'Php app ready' >> SERVICE_PREFIX") + DEFAULT_SERVICE_LOG_FILE + " 2>&1" + "\"}\n}";

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            logInfo() << "Starting server application";
            QFile::remove(QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + serviceName + START_TRIGGER_FILE);
            touch(QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + serviceName + START_TRIGGER_FILE);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Launching service using newly generated igniter.";
            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case NoType: {
            logError() << "No web application detected in service directory:" << servicePath;
            raise(SIGTERM);

        } break;
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

    /* NOTE: make sure TheSS is running for user, and launch it if it's not: */
    bool ok = false, fail = false;
    QString ssPidFile = getHomeDir() + "/." + getenv("USER") + ".pid";
    QString aPid = readFileContents(ssPidFile).trimmed();
    uint pid = aPid.toInt(&ok, 10);
    if (ok) {
        if (not pidIsAlive(pid)) {
            logWarn() << "No alive pid of Service Spawner found. It will be started for user:" << getenv("USER");
            fail = true;
        }
    } else {
        logWarn() << "Pid file is damaged or doesn't contains valid pid. File will be removed, and ss restarted.";
        QFile::remove(ssPidFile);
        fail = true;
    }
    if (fail) {
        launchServiceSpawner();
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
    auto diskMap = getDiskFree(getenv("HOME"));
    Q_FOREACH(auto map, diskMap.keys()) {
        auto value = diskMap.take(map);
        if (value < MINIMUM_DISK_SPACE_IN_MEGS) {
            logError() << "Insufficient disk space (less than " << QString::number(MINIMUM_DISK_SPACE_IN_MEGS) << "MiB) detected on remote destination machine. Deploy aborted!";
            raise(SIGTERM);

        } else
            logInfo() << "Sufficient disk space detected for:" << map << "(" << QString::number(value) << "MiB)";
    }

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