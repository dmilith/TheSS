/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "webapp_watcher.h"
#include "utils.h"


SvdWebAppHookTriggerFiles::SvdWebAppHookTriggerFiles(const QString& path) {
    start     = new SvdHookTriggerFile(path + "/.start");
    stop      = new SvdHookTriggerFile(path + "/.stop");
    restart   = new SvdHookTriggerFile(path + "/.restart");
    reload    = new SvdHookTriggerFile(path + "/.reload");
}


SvdWebAppHookTriggerFiles::~SvdWebAppHookTriggerFiles() {
    delete start;
    delete stop;
    delete restart;
    delete reload;
}


SvdWebAppHookIndicatorFiles::SvdWebAppHookIndicatorFiles(const QString& path) {
    autostart   = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_AUTOSTART_FILE);
    running     = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_RUNNING_FILE);
}


SvdWebAppHookIndicatorFiles::~SvdWebAppHookIndicatorFiles() {
    delete autostart;
    delete running;
}


SvdWebAppWatcher::SvdWebAppWatcher(const QString& domain) {
    logDebug() << "Starting SvdWebAppWatcher for domain:" << domain;

    webAppDir = getWebAppsDir() + "/" + domain;

    webApp = new SvdWebAppDeployer(domain);

    fileEvents = new SvdFileEventsManager();
    fileEvents->registerFile(webAppDir);

    triggerFiles = new SvdWebAppHookTriggerFiles(webAppDir);
    indicatorFiles = new SvdWebAppHookIndicatorFiles(webAppDir);

    /* connect file event slots to watcher: */
    connect(fileEvents, SIGNAL(directoryChanged(QString)), this, SLOT(dirChangedSlot(QString)));
    connect(fileEvents, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));

    /* connect watcher signals to slots of web application deployer: */
    connect(this, SIGNAL(startWebApp()), webApp, SLOT(startSlot()));
    connect(this, SIGNAL(stopWebApp()), webApp, SLOT(stopSlot()));
    connect(this, SIGNAL(restartWebApp()), webApp, SLOT(restartSlot()));
    connect(this, SIGNAL(reloadWebApp()), webApp, SLOT(reloadSlot()));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));

    /* manage web application autostart */
    if (indicatorFiles->autostart->exists()) {
        logInfo() << "Performing autostart of WebApplication domain:" << domain;
        emit startWebApp();
    }
}


void SvdWebAppWatcher::shutdownSlot() {
    qDebug() << "Invoked shutdown slot.";
    qDebug() << "Emitting stopWebApp signal.";
    emit stopWebApp();
}


void SvdWebAppWatcher::dirChangedSlot(const QString& dir) {
    logTrace() << "Directory changed:" << dir;


    /* reload */
    if (triggerFiles->reload->exists()) {
        triggerFiles->reload->remove();
        logDebug() << "Emitting reloadWebApp() signal.";
        emit reloadWebApp();
        return;
    }

    /* start */
    if (triggerFiles->start->exists()) {
        triggerFiles->start->remove();
        if (indicatorFiles->running->exists())
            logWarn() << "Interrupted emission of startWebApp() signal. Domain is already running.";
        else {
            logDebug() << "Emitting startWebApp() signal.";
            emit startWebApp();
        }
        return;
    }

    /* stop */
    if (triggerFiles->stop->exists()) {
        triggerFiles->stop->remove();
        if (indicatorFiles->running->exists()) {
            logDebug() << "Emitting stopWebApp() signal.";
            emit stopWebApp();
        } else
            logWarn() << "Interrupted emission of stopWebApp() signal. Domain is not running.";
        return;
    }

    /* restart */
    if (triggerFiles->restart->exists()) {
        triggerFiles->restart->remove();
        logDebug() << "Emitting restartWebApp() signal.";
        emit restartWebApp();
        return;
    }

    /* Check if directory wasn't removed */
    if (not QFile::exists(dir)) {
        logWarn() << "Web application directory was removed. Shutting down.";
        emit shutdownSlot();
    }

}


void SvdWebAppWatcher::fileChangedSlot(const QString& file) {
    logDebug() << "File changed:" << file;

}


SvdWebAppWatcher::~SvdWebAppWatcher() {
    delete fileEvents;
    delete triggerFiles;
    delete indicatorFiles;
    delete webApp;
}
