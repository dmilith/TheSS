/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "../service_spawner/utils.h"
#include "public_watcher.h"


SvdPublicWatcher::SvdPublicWatcher() {
    logDebug() << "Starting SvdPublicWatcher";

    fileEvents = new SvdFileEventsManager();
    entries = QDir(DEFAULT_PUBLIC_DIR).entryList(QDir::Files, QDir::Time).toSet();

    /* connect file event slots to watcher: */
    connect(fileEvents, SIGNAL(directoryChanged(QString)), this, SLOT(dirChangedSlot(QString)));
    connect(fileEvents, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));
    fileEvents->registerFile(DEFAULT_PUBLIC_DIR);
    Q_FOREACH(auto file, entries) {
        logDebug() << "Putting watch on file:" << file;
        fileEvents->registerFile(QString(DEFAULT_PUBLIC_DIR) + "/" + file);
    }

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));
    reindexPublicDir();
}


void SvdPublicWatcher::reindexPublicDir() {
    logDebug() << "Old set of ENTRIES:" << this->entries;
    QSet<QString> currentEntries = QDir(DEFAULT_PUBLIC_DIR).entryList(QDir::Files, QDir::Time).toSet();
    QSet<QString> newEntries = currentEntries.subtract(entries);
    logInfo() << "Detected new entries:" << newEntries;
    processEntries(newEntries);
}


void SvdPublicWatcher::processEntries(QSet<QString> newEntries) {
    if (newEntries.isEmpty())
        logDebug() << "Empty list to process. Skipped";
    else {
        Q_FOREACH(auto entry, newEntries) {
            logInfo() << "Processing entry:" << entry;
            validateDomainExistanceFor(entry);
        }
    }
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
    validateDomainExistanceFor(file);
}


void SvdPublicWatcher::validateDomainExistanceFor(QString file) {
    auto invokedFile = file.replace(QString(DEFAULT_PUBLIC_DIR) + "/", "");
    if (not invokedFile.contains("_")) {
        logWarn() << "Skipped check for file:" << invokedFile;
        return;
    }
    auto serviceName = invokedFile.split("_").at(0);
    auto userName = invokedFile.split("_").at(1);
    logDebug() << "Username:" << userName;
    logDebug() << "ServiceName:" << serviceName;
    if (userName.endsWith(".web-app-request")) {
        logWarn() << "Skipping invalid file in Public dir:" << invokedFile;
        return;
    }
    logInfo() << "Validating service:" << serviceName << "for user:" << userName;
    auto root = "/Users/" + userName + SOFTWARE_DATA_DIR;
    auto serviceBase = root + "/" + serviceName;
    auto aFile = serviceBase + DEFAULT_SERVICE_CONFIGURED_FILE;
    logDebug() << "Validating existance of:" << aFile << "?-" << QFile::exists(aFile);
    auto fileContent = readFileContents(QString(DEFAULT_PUBLIC_DIR) + "/" +file).trimmed();
    Q_FOREACH(auto entry, entries) {
        auto entryContents = readFileContents(QString(DEFAULT_PUBLIC_DIR) + "/" + entry).trimmed();
        if (entry != file) { /* don't check same file */
            logDebug() << "Trying to compare:" << entryContents << "with" << fileContent << "(" << entry << "?" << file << ")";
            if (entryContents == fileContent) {
                QSet<QString> remFiles;
                remFiles << QString(DEFAULT_PUBLIC_DIR) + "/" + file;
                remFiles << serviceBase + "/proxy.conf";
                logError() << "Entries files contain domain:" << fileContent << "Files:" << remFiles << " will be removed!";
                auto notificationRoot = root + NOTIFICATIONS_DATA_DIR;
                auto notificationFile = notificationRoot + "/duplicated_domain.error";
                auto notificationContents = "Domain check failed! Domain already taken: " + fileContent;
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
            }
        }
    }
}


void SvdPublicWatcher::dirChangedSlot(const QString& dir) {
    logInfo() << "Directory changed:" << dir;

    if (dir == DEFAULT_PUBLIC_DIR) {
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
