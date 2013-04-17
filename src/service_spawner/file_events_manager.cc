/**
 *  @author tallica
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "file_events_manager.h"
#include "utils.h"


void SvdFileEventsManager::registerFile(const QString& path) {
    if (QFile::exists(path)) {
        logDebug() << "Registering watcher on an existing file:" << path;
        addPath(path);
    } else {
        logTrace() << "File to watch does not exist. Assumming that we want to monitor a directory:" << path;
        QDir().mkpath(path);
        logDebug() << "Creating and registering watcher on a new directory:" << path;
        addPath(path);
    }
}


void SvdFileEventsManager::unregisterFile(const QString& path) {
    logDebug() << "Unregistering watcher on file:" << path;
    removePath(path);
}


bool SvdFileEventsManager::isWatchingFile(const QString& path) {
    return files().contains(path);
}


bool SvdFileEventsManager::isWatchingDir(const QString& path) {
    return directories().contains(path);
}
