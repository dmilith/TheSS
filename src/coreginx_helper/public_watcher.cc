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
            validateDomainExistanceFor(QString(DEFAULT_PUBLIC_DIR) + "/" + entry);
            // TODO check if domain exists or not in cwd
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
    // reindexPublicDir(); /* we don't need to reload dir entries, cause nothing has changed there if just file was changed */
    reindexPublicDir();
    validateDomainExistanceFor(file);
}


void SvdPublicWatcher::validateDomainExistanceFor(const QString& file) {
    auto fileContent = readFileContents(file).trimmed();
    Q_FOREACH(auto entry, entries) {
        auto entryContents = readFileContents(QString(DEFAULT_PUBLIC_DIR) + "/" + entry).trimmed();
        logDebug() << "Trying to compare:" << entryContents << "with" << fileContent;
        if (entryContents.contains(fileContent)) {
            logError() << "Entries files contain domain:" << fileContent;

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
