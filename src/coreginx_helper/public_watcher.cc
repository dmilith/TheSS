/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "../service_spawner/utils.h"
#include "public_watcher.h"


// SvdUserHookTriggerFiles::SvdUserHookTriggerFiles(const QString& path) {
//     shutdown = new SvdHookTriggerFile(path + DEFAULT_SS_SHUTDOWN_HOOK_FILE);
//     graceShutdown = new SvdHookTriggerFile(path + DEFAULT_SS_GRACEFUL_SHUTDOWN_HOOK_FILE);
// }


// SvdUserHookTriggerFiles::~SvdUserHookTriggerFiles() {
//     delete shutdown;
//     delete graceShutdown;
// }


// SvdUserHookIndicatorFiles::SvdUserHookIndicatorFiles(const QString& path) {
//     autostart = new SvdHookIndicatorFile(path + AUTOSTART_TRIGGER_FILE);
// }


// SvdUserHookIndicatorFiles::~SvdUserHookIndicatorFiles() {
//     delete autostart;
// }



SvdPublicWatcher::SvdPublicWatcher() {
    logDebug() << "Starting SvdPublicWatcher";

    fileEvents = new SvdFileEventsManager();
    fileEvents->registerFile(DEFAULT_PUBLIC_DIR);

    /* connect file event slots to watcher: */
    connect(fileEvents, SIGNAL(directoryChanged(QString)), this, SLOT(dirChangedSlot(QString)));
    connect(fileEvents, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));
}



void SvdPublicWatcher::shutdownSlot() {
    QString lockName = getHomeDir() + "/..pid";
    lockName = getHomeDir() + "/.coreginx_helper.pid";
    logDebug() << "Removing lock file:" << lockName;
    QFile::remove(lockName);
    logInfo() << "Shutdown completed.";
}


void SvdPublicWatcher::invokeFileChangedTrigger() {
    logInfo() << "Invoked file changed trigger. Performing reindex of Public dir";

}


void SvdPublicWatcher::dirChangedSlot(const QString& dir) {
    logInfo() << "Directory changed:" << dir;

    if (dir == DEFAULT_PUBLIC_DIR) {
        logTrace() << "Invoked trigger in dir:" << dir;
        invokeFileChangedTrigger();
        return;
    }

}


void SvdPublicWatcher::fileChangedSlot(const QString& file) {
    logInfo() << "File changed:" << file;
}


SvdPublicWatcher::~SvdPublicWatcher() {
    disconnect(fileEvents, SIGNAL(directoryChanged(QString)));
    disconnect(fileEvents, SIGNAL(fileChanged(QString)));
    fileEvents->unregisterFile(DEFAULT_PUBLIC_DIR);
}
