/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "../core/utils.h"
#include "public_watcher.h"


SvdPublicWatcher::SvdPublicWatcher() {
    logDebug() << "Starting SvdPublicWatcher";

    fileEvents = new SvdFileEventsManager();
    fileEntries = QDir(DEFAULT_PUBLIC_DIR).entryList(QDir::Files, QDir::Time).toSet();
    loadExistingDomains();

    /* connect file event slots to watcher: */
    connect(fileEvents, SIGNAL(directoryChanged(QString)), this, SLOT(dirChangedSlot(QString)));
    connect(fileEvents, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));
    fileEvents->registerFile(DEFAULT_PUBLIC_DIR);
    Q_FOREACH(QString file, fileEntries) {
        logDebug() << "Putting watch on file:" << file;
        fileEvents->registerFile(QString(DEFAULT_PUBLIC_DIR) + file);
    }

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));
    reindexPublicDir();
}


void SvdPublicWatcher::loadExistingDomains() {
    loadExistingMutex.lock();
    Q_FOREACH(QString entry, fileEntries) {
        domains << readFileContents(QString(DEFAULT_PUBLIC_DIR) + entry).trimmed();
    }
    logDebug() << "Loaded domains:" << domains;
    loadExistingMutex.unlock();
}


void SvdPublicWatcher::reindexPublicDir() {
    publicReindexMutex.lock();
    logDebug() << "Old set of ENTRIES:" << this->fileEntries;
    QSet<QString> currentEntries = QDir(DEFAULT_PUBLIC_DIR).entryList(QDir::Files, QDir::Time).toSet();
    QSet<QString> newEntries = currentEntries.subtract(fileEntries);
    logDebug() << "Detected new fileEntries:" << newEntries;
    if (not newEntries.isEmpty())
        Q_FOREACH(auto entry, newEntries) {
            logInfo() << "Processing entry:" << entry;
            validateDomainExistanceFor(entry);
        }
    fileEntries = QDir(DEFAULT_PUBLIC_DIR).entryList(QDir::Files, QDir::Time).toSet();
    publicReindexMutex.unlock();
}


void SvdPublicWatcher::shutdownSlot() {
    QString lockName = getHomeDir() + "/.coreginx_helper.pid";
    logDebug() << "Removing lock file:" << lockName;
    QFile::remove(lockName);
    logInfo() << "Shutdown completed.";
}


void SvdPublicWatcher::invokeDirChangedTrigger() {
    logInfo() << "Invoked dir changed trigger. Performing reindex of Public dir";
    reindexPublicDir();
}


void SvdPublicWatcher::invokeFileChangedTrigger(const QString& file) {
    logInfo() << "Invoked file changed trigger of file:" << file;
    reindexPublicDir();
    loadExistingDomains();
    validateDomainExistanceFor(file);
}


void SvdPublicWatcher::validateDomainExistanceFor(QString file) {
    auto invokedFile = file.replace(QString(DEFAULT_PUBLIC_DIR), "");
    if (not invokedFile.contains("_")) {
        logWarn() << "Skipped check for file:" << invokedFile;
        return;
    }
    auto serviceName = invokedFile.split("_").at(0);
    auto userName = invokedFile.split("_").at(1);
    logDebug() << "Username:" << userName;
    logDebug() << "ServiceName:" << serviceName;
    userName = userName.replace(".web-app", "");
    if (userName.isEmpty()) {
        logError() << "Empty user name given?";
        raise(SIGTERM);
    }

    // logInfo() << "Validating service:" << serviceName << "for user:" << userName;
    auto root = QString(DEFAULT_HOME_DIR) + userName + SOFTWARE_DATA_DIR;
    auto serviceBase = root + "/" + serviceName;

    // /* XXX: it's almost raw service config implementation */
    // auto ignitersDir = QString(DEFAULT_HOME_DIR) + userName + DEFAULT_USER_IGNITERS_DIR;
    // auto igniterSourceFile = ignitersDir + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
    // auto igniterSource = readFileContents(igniterSourceFile).trimmed();

    // auto servicePortsDir = serviceBase + DEFAULT_SERVICE_PORTS_DIR;
    // auto servicePortFile = servicePortsDir + DEFAULT_SERVICE_PORT_NUMBER;
    // auto servicePort = readFileContents(servicePortFile).trimmed();

    // auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
    // hash->addData(igniterSource.toUtf8(), igniterSource.length());
    // auto igniterSha1Hash = hash->result().toHex();
    // delete hash;
    // auto sha = igniterSha1Hash.right(20) + "." + servicePort;
    // auto aFile = serviceBase + DEFAULT_SERVICE_CONFS_DIR + sha + DEFAULT_SERVICE_CONFIGURED_FILE;
    // bool mayProceed = QFile::exists(aFile);

    // if (mayProceed)
    //     logInfo() << "Service is configured;" << serviceName;
    auto aPublicFile = QString(DEFAULT_PUBLIC_DIR) + file;
    auto fileContent = readFileContents(aPublicFile).trimmed();
    if (not QFile::exists(aPublicFile)) {
        logInfo() << "Detected first deploy of web-app:" << serviceName << "for user:" << userName;
        commitDeploy(serviceName);
    } else
    if (not fileContent.isEmpty()) {
        logDebug() << "Trying to find existing domain:" << fileContent << "(in:" << file << "), existing domains:" << domains;
        if (domains.contains(fileContent) and not fileEntries.contains(file)) {
            QSet<QString> remFiles;
            remFiles << QString(DEFAULT_PUBLIC_DIR) + file;
            remFiles << serviceBase + "/proxy.conf";
            logError() << "Entries files contain domain:" << fileContent << "Files:" << remFiles << " will be removed!";
            auto notificationRoot = root + NOTIFICATIONS_DATA_DIR;
            auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
            hash->addData(invokedFile.toUtf8(), invokedFile.length());
            auto notificationFile = notificationRoot + "/" + hash->result().toHex() + "-duplicated_domain.error";
            auto notificationContents = "Domain check failed for web-app: " + serviceName + ", user:" + userName + "! Domain already taken: " + fileContent;
            logDebug() << "Writing to:" << notificationFile << "with content:" << notificationContents;
            if (QDir(notificationRoot).exists()) {
                writeToFile(notificationFile, notificationContents);
                logInfo() << "Error Notification created!";
            } else {
                logWarn() << "Can't create notification in non existant directory:" << notificationRoot;
            }
            Q_FOREACH(auto el, remFiles) {
                logDebug() << "Removing:" << el;
                QFile::remove(el);
            }
        } else { /* redeploy of same app case */
            logInfo() << "Detected redeploy of web-app:" << serviceName << "for user:" << userName;
            commitDeploy(serviceName);
        }
    } else
        logDebug() << "File contents empty.";
}


void SvdPublicWatcher::commitDeploy(const QString& serviceName) {
    auto coreginxReloadTrigger = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR + "/Coreginx" + RELOAD_TRIGGER_FILE;
    logDebug() << "Creating reload request for Coreginx service:" << coreginxReloadTrigger;
    QFile::remove(coreginxReloadTrigger); /* sanity check */
    touch(coreginxReloadTrigger);
    logInfo() << "Coreginx reload was triggered for request from service:" << serviceName;
}


void SvdPublicWatcher::dirChangedSlot(const QString& dir) {
    logInfo() << "Directory changed:" << dir;

    if (QString(DEFAULT_PUBLIC_DIR).contains(dir)) {
        logTrace() << "Invoked trigger in dir:" << dir;
        invokeDirChangedTrigger();
        return;
    }

}


void SvdPublicWatcher::fileChangedSlot(const QString& file) {
    logInfo() << "File changed:" << file;
    invokeFileChangedTrigger(file);
}


SvdPublicWatcher::~SvdPublicWatcher() {
    disconnect(fileEvents, SIGNAL(directoryChanged(QString)));
    disconnect(fileEvents, SIGNAL(fileChanged(QString)));
    fileEvents->unregisterFile(DEFAULT_PUBLIC_DIR);
}
