/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "user_watcher.h"
#include "../core/utils.h"
// #include "webapp_deployer.h"


SvdUserHookTriggerFiles::SvdUserHookTriggerFiles(const QString& path) {
    shutdown = new SvdHookTriggerFile(path + DEFAULT_SS_SHUTDOWN_HOOK_FILE);
    graceShutdown = new SvdHookTriggerFile(path + DEFAULT_SS_GRACEFUL_SHUTDOWN_HOOK_FILE);
}


SvdUserHookTriggerFiles::~SvdUserHookTriggerFiles() {
    delete shutdown;
    delete graceShutdown;
}


SvdUserHookIndicatorFiles::SvdUserHookIndicatorFiles(const QString& path) {
    autostart = new SvdHookIndicatorFile(path + AUTOSTART_TRIGGER_FILE);
}


SvdUserHookIndicatorFiles::~SvdUserHookIndicatorFiles() {
    delete autostart;
}


void SvdUserWatcher::init(uid_t uid) {
    logDebug() << "Starting SvdUserWatcher for user:" << getenv("USER");

    this->uid = uid;
    this->homeDir = getHomeDir(uid);
    this->softwareDataDir = getSoftwareDataDir(uid);
    // this->dataCollector = new SvdDataCollector();

    collectServices();
    // collectWebApplications();

    fileEvents = new SvdFileEventsManager();
    fileEvents->registerFile(homeDir);
    fileEvents->registerFile(softwareDataDir);
    // fileEvents->registerFile(homeDir + DEFAULT_WEBAPPS_DIR);

    triggerFiles = new SvdUserHookTriggerFiles(homeDir);
    indicatorFiles = new SvdUserHookIndicatorFiles(homeDir);

    /* connect file event slots to watcher: */
    connect(fileEvents, SIGNAL(directoryChanged(QString)), this, SLOT(dirChangedSlot(QString)));
    connect(fileEvents, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));

    // if (QFile::exists(homeDir + DEFAULT_SS_PROCESS_DATA_COLLECTION_HOOK_FILE)) {
    //     logInfo() << "Found data collector trigger file. Launching data collector for all user processes";

    //     /* launch new collector service */
    //     QString name = "ProcessDataCollector";
    //     auto config = new SvdServiceConfig(name);

    //     QDir().mkdir(config->prefixDir());
    //     if (not QFile::exists(config->prefixDir() + "/.autostart")) {
    //         touch(config->prefixDir() + "/.autostart");
    //         touch(config->prefixDir() + "/.start");
    //     }

    //     delete config;
    // }

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));
}


// void SvdUserWatcher::collectWebApplications() {

//     logDebug() << "Looking for WebApps inside" << homeDir + DEFAULT_WEBAPPS_DIR;
//     QStringList oldWebApps = webApps;
//     logDebug() << "Previous list of WebApps:" << oldWebApps;
//     webApps = QDir(homeDir + DEFAULT_WEBAPPS_DIR).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
//     logDebug() << "Current list of WebApps:" << webApps;

//     Q_FOREACH(QString domain, webApps) {
//         if (not oldWebApps.contains(domain)) {
//             qDebug() << "Found WebApp:" << domain;
//             this->webAppWatchers << new SvdWebAppWatcher(domain);
//         }
//     }
// }


void SvdUserWatcher::collectServices() {
    collectorMutex.lock();
    logDebug() << "Looking for services inside" << softwareDataDir;
    QStringList oldServices = services;
    logDebug() << "Previous list of services:" << oldServices;
    services = QDir(softwareDataDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    logDebug() << "Current list of services:" << services;

    Q_FOREACH(QString name, services) {
        if (QFile::exists(homeDir + DEFAULT_USER_IGNITERS_DIR + name + DEFAULT_SOFTWARE_TEMPLATE_EXT)) {
            logDebug() << "Found igniter for service:" << name;
            if (not oldServices.contains(name) and not name.endsWith(DEFAULT_SERVICE_DISABLED_POSTFIX)) {
                logInfo() << "Initializing watchers for data dir of service:" << name;

                /* detect user watcher */
                if (this->serviceWatchers.length() == 0) {
                    logDebug() << "Adding new service watcher:" << name;
                    this->serviceWatchers << new SvdServiceWatcher(name);
                } else {
                    for (int ind = 0; ind < this->serviceWatchers.length(); ind++) {
                        logDebug() << "Comparing:" << serviceWatchers.at(ind)->name() << "and" << name;
                        if (name == serviceWatchers.at(ind)->name()) {
                            logDebug() << "Found old user watcher! Replacing!";
                            serviceWatchers.at(ind)->deleteLater();
                            this->serviceWatchers.removeAt(ind);
                        }
                    }
                    this->serviceWatchers << new SvdServiceWatcher(name);
                }
            }
        }
    }
    collectorMutex.unlock();
}


SvdUserWatcher::SvdUserWatcher() {
    init(getuid());
}


SvdUserWatcher::SvdUserWatcher(uid_t uid) {
    init(uid);
}


void SvdUserWatcher::shutdownSlot() {
    QString lockName = getHomeDir() + "/." + getenv("USER") + ".pid";
    if (uid == 0) {
        lockName = getHomeDir() + "/.root.pid";

        /* remove user pid files */
        // Q_FOREACH(int userUid, gatherUserUids()) {
        //     QString lockName = QString(USERS_HOME_DIR) + "/" + QString::number(userUid) + "/." + QString::number(userUid) + ".pid";
        //     logDebug() << "Removing lock file:" << lockName;
        //     QFile::remove(lockName);
        // }
        SvdService::shutdownDefaultVPNNetwork();
    }
    logDebug() << "Removing lock file:" << lockName;
    QFile::remove(lockName);
    QFile::remove(getHomeDir() + DEFAULT_SS_SHUTDOWN_HOOK_FILE);
    QFile::remove(getHomeDir() + DEFAULT_SS_GRACEFUL_SHUTDOWN_HOOK_FILE);
    logInfo() << "Shutdown completed.";
}


void SvdUserWatcher::checkUserControlTriggers() {
    if (QFile::exists(homeDir + DEFAULT_SS_GRACEFUL_SHUTDOWN_HOOK_FILE)) {
        QString msg = "Invoked graceful shutdown trigger. Sending SS down with services.";
        notification(msg, WARNING);
        QFile::remove(homeDir + DEFAULT_SS_GRACEFUL_SHUTDOWN_HOOK_FILE);
        /* and remove pid file */
        if (getuid() == 0) {
            QFile::remove(homeDir + "/.root.pid");
            SvdService::shutdownDefaultVPNNetwork();
        } else
            QFile::remove(homeDir + "/." + getenv("USER") + ".pid");

        Q_FOREACH(SvdServiceWatcher *svce, serviceWatchers) {
            if (svce)
                svce->shutdownSlot();
        }
        logInfo() << "Graceful shutdown";
        exit(EXIT_SUCCESS);
    }
    if (QFile::exists(homeDir + DEFAULT_SS_SHUTDOWN_HOOK_FILE)) {
        QString msg = "Invoked shutdown trigger. Sending SS down.";
        notification(msg);
        QFile::remove(homeDir + DEFAULT_SS_SHUTDOWN_HOOK_FILE);
        /* and remove pid file */
        if (getuid() == 0) {
            SvdService::shutdownDefaultVPNNetwork();
            QFile::remove(homeDir + "/.root.pid");
        } else
            QFile::remove(homeDir + "/." + getenv("USER") + ".pid");

        raise(SIGTERM);
    }
}


void SvdUserWatcher::dirChangedSlot(const QString& dir) {
    logTrace() << "Directory changed:" << dir;

    if (dir == homeDir) {
        logTrace() << "Invoked trigger in dir:" << dir;
        checkUserControlTriggers();
        return;
    }

    // if (dir.contains(DEFAULT_WEBAPPS_DIR)) {
    //     logInfo() << "Deployer has been triggered with new web application";
    //     collectWebApplications();
    //     return;
    // }

    if (dir == softwareDataDir) {
        collectServices();
        return;
    }
}


void SvdUserWatcher::fileChangedSlot(const QString& file) {
    logDebug() << "File changed:" << file;
}


SvdUserWatcher::~SvdUserWatcher() {
    delete triggerFiles;
    delete indicatorFiles;

    disconnect(fileEvents, SIGNAL(directoryChanged(QString)));
    disconnect(fileEvents, SIGNAL(fileChanged(QString)));
    fileEvents->unregisterFile(homeDir);
    fileEvents->unregisterFile(softwareDataDir);
}
