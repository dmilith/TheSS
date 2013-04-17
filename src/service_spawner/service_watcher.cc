/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "service_watcher.h"
#include "utils.h"


SvdHookTriggerFiles::SvdHookTriggerFiles(const QString& path) {
    install   = new SvdHookTriggerFile(path + "/.install");
    configure = new SvdHookTriggerFile(path + "/.configure");
    start     = new SvdHookTriggerFile(path + "/.start");
    stop      = new SvdHookTriggerFile(path + "/.stop");
    afterStart= new SvdHookTriggerFile(path + "/.afterStart");
    afterStop = new SvdHookTriggerFile(path + "/.afterStop");
    restart   = new SvdHookTriggerFile(path + "/.restart");
    reload    = new SvdHookTriggerFile(path + "/.reload");
    validate  = new SvdHookTriggerFile(path + "/.validate");
}


SvdHookTriggerFiles::~SvdHookTriggerFiles() {
    delete install;
    delete configure;
    delete start;
    delete afterStart;
    delete stop;
    delete afterStop;
    delete restart;
    delete reload;
    delete validate;
}


SvdHookIndicatorFiles::SvdHookIndicatorFiles(const QString& path) {
    autostart       = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_AUTOSTART_FILE);
    afterStarting   = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_AFTERSTARTING_FILE);
    afterStopping   = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_AFTERSTOPPING_FILE);
    running         = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_RUNNING_FILE);
    installing      = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_INSTALLING_FILE);
    configuring     = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_CONFIGURING_FILE);
    reloading       = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_RELOADING_FILE);
    validating      = new SvdHookIndicatorFile(path + DEFAULT_SERVICE_VALIDATING_FILE);
}


SvdHookIndicatorFiles::~SvdHookIndicatorFiles() {
    delete autostart;
    delete afterStarting;
    delete afterStopping;
    delete running;
    delete installing;
    delete configuring;
    delete reloading;
    delete validating;
}


SvdServiceWatcher::SvdServiceWatcher(const QString& name) {
    logDebug() << "Starting SvdServiceWatcher for service:" << name;

    dataDir = getServiceDataDir(name);

    service = new SvdService(name);

    fileEvents = new SvdFileEventsManager();
    fileEvents->registerFile(dataDir);

    triggerFiles = new SvdHookTriggerFiles(dataDir);
    indicatorFiles = new SvdHookIndicatorFiles(dataDir);

    /* connect file event slots to watcher: */
    connect(fileEvents, SIGNAL(directoryChanged(QString)), this, SLOT(dirChangedSlot(QString)));
    connect(fileEvents, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));

    /* connect watcher signals to slots of service: */
    connect(this, SIGNAL(installService()), service, SLOT(installSlot()));
    connect(this, SIGNAL(configureService()), service, SLOT(configureSlot()));
    connect(this, SIGNAL(validateService()), service, SLOT(validateSlot()));
    connect(this, SIGNAL(startService()), service, SLOT(startSlot()));
    connect(this, SIGNAL(afterStartService()), service, SLOT(afterStartSlot()));
    connect(this, SIGNAL(stopService()), service, SLOT(stopSlot()));
    connect(this, SIGNAL(afterStopService()), service, SLOT(afterStopSlot()));
    connect(this, SIGNAL(restartService()), service, SLOT(restartSlot()));
    connect(this, SIGNAL(reloadService()), service, SLOT(reloadSlot()));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));

    /* manage service autostart */
    if (indicatorFiles->autostart->exists()) {
        logInfo() << "Performing autostart of service:" << name;
        emit startService();
    }
}


void SvdServiceWatcher::shutdownSlot() {
    qDebug() << "Invoked shutdown slot.";
    qDebug() << "Emitting stopService signal.";
    emit stopService();
}


void SvdServiceWatcher::dirChangedSlot(const QString& dir) {
    logTrace() << "Directory changed:" << dir;

    /* configure */
    if (triggerFiles->configure->exists()) {
        triggerFiles->configure->remove();
        logDebug() << "Emitting configureService() signal.";
        emit configureService();
        return;
    }

    /* validate */
    if (triggerFiles->validate->exists()) {
        triggerFiles->validate->remove();
        logDebug() << "Emitting validateService() signal.";
        emit validateService();
        return;
    }

    /* reload */
    if (triggerFiles->reload->exists()) {
        triggerFiles->reload->remove();
        logDebug() << "Emitting reloadService() signal.";
        emit reloadService();
        return;
    }

    /* install */
    if (triggerFiles->install->exists()) {
        triggerFiles->install->remove();
        if (indicatorFiles->installing->exists())
            logWarn() << "Interrupted emission of installService() signal. Service is currently installing.";
        else {
            logDebug() << "Emitting installService() signal.";
            emit installService();
        }
        return;
    }

    /* start */
    if (triggerFiles->start->exists()) {
        triggerFiles->start->remove();
        if (indicatorFiles->running->exists())
            logWarn() << "Interrupted emission of startService() signal. Service is already running.";
        else {
            logDebug() << "Emitting startService() signal.";
            emit startService();
        }
        return;
    }

    /* afterStart */
    if (triggerFiles->afterStart->exists()) {
        triggerFiles->afterStart->remove();
        logDebug() << "Emitting afterStartService() signal.";
        emit afterStartService();
        return;
    }

    /* stop */
    if (triggerFiles->stop->exists()) {
        triggerFiles->stop->remove();
        if (indicatorFiles->running->exists()) {
            logDebug() << "Emitting stopService() signal.";
            emit stopService();
        } else
            logWarn() << "Interrupted emission of stopService() signal. Service is not running.";
        return;
    }

    /* afterStop */
    if (triggerFiles->afterStop->exists()) {
        triggerFiles->afterStop->remove();
        logDebug() << "Emitting afterStopService() signal.";
        emit afterStopService();
        return;
    }

    /* restart */
    if (triggerFiles->restart->exists()) {
        triggerFiles->restart->remove();
        logDebug() << "Emitting restartService() signal.";
        emit restartService();
        return;
    }

    /* Check if directory wasn't removed */
    if (not QFile::exists(dir)) {
        logWarn() << "Service data directory was removed. Shutting down service watcher.";
        emit shutdownSlot();
    }

}


void SvdServiceWatcher::fileChangedSlot(const QString& file) {
    logDebug() << "File changed:" << file;

}


SvdServiceWatcher::~SvdServiceWatcher() {
    delete fileEvents;
    delete triggerFiles;
    delete indicatorFiles;
    delete service;
}
