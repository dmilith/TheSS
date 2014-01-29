/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "service_watcher.h"
#include "utils.h"


SvdHookTriggerFiles::SvdHookTriggerFiles(const QString& path) {
    install                = new SvdHookTriggerFile(path + INSTALL_TRIGGER_FILE);
    configure              = new SvdHookTriggerFile(path + CONFIGURE_TRIGGER_FILE);
    reConfigure            = new SvdHookTriggerFile(path + RECONFIGURE_TRIGGER_FILE);
    reConfigureWithoutDeps = new SvdHookTriggerFile(path + RECONFIGURE_WITHOUT_DEPS_TRIGGER_FILE);
    start                  = new SvdHookTriggerFile(path + START_TRIGGER_FILE);
    startWithoutDeps       = new SvdHookTriggerFile(path + START_WITHOUT_DEPS_TRIGGER_FILE);
    stop                   = new SvdHookTriggerFile(path + STOP_TRIGGER_FILE);
    stopWithoutDeps        = new SvdHookTriggerFile(path + STOP_WITHOUT_DEPS_TRIGGER_FILE);
    afterStart             = new SvdHookTriggerFile(path + AFTERSTART_TRIGGER_FILE);
    afterStop              = new SvdHookTriggerFile(path + AFTERSTOP_TRIGGER_FILE);
    restart                = new SvdHookTriggerFile(path + RESTART_TRIGGER_FILE);
    restartWithoutDeps     = new SvdHookTriggerFile(path + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
    reload                 = new SvdHookTriggerFile(path + RELOAD_TRIGGER_FILE);
    validate               = new SvdHookTriggerFile(path + VALIDATE_TRIGGER_FILE);
}


SvdHookTriggerFiles::~SvdHookTriggerFiles() {
    delete install;
    delete configure;
    delete reConfigure;
    delete start;
    delete startWithoutDeps;
    delete afterStart;
    delete stop;
    delete stopWithoutDeps;
    delete afterStop;
    delete restart;
    delete restartWithoutDeps;
    delete reload;
    delete validate;
}


SvdHookIndicatorFiles::SvdHookIndicatorFiles(const QString& path) {
    autostart       = new SvdHookIndicatorFile(path + AUTOSTART_TRIGGER_FILE);
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


QString SvdServiceWatcher::name() {
    return appName;
}


SvdServiceWatcher::SvdServiceWatcher(const QString& name) {
    logDebug() << "Starting SvdServiceWatcher for service:" << name;

    appName = name;

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
    connect(this, SIGNAL(reConfigureService()), service, SLOT(reConfigureSlot()));
    connect(this, SIGNAL(reConfigureWithoutDepsService()), service, SLOT(reConfigureWithoutDepsSlot()));
    connect(this, SIGNAL(validateService()), service, SLOT(validateSlot()));
    connect(this, SIGNAL(startService()), service, SLOT(startSlot()));
    connect(this, SIGNAL(startWithoutDepsService()), service, SLOT(startWithoutDepsSlot()));
    connect(this, SIGNAL(afterStartService()), service, SLOT(afterStartSlot()));
    connect(this, SIGNAL(stopService()), service, SLOT(stopSlot()));
    connect(this, SIGNAL(stopWithoutDepsService()), service, SLOT(stopWithoutDepsSlot()));
    connect(this, SIGNAL(afterStopService()), service, SLOT(afterStopSlot()));
    connect(this, SIGNAL(restartService()), service, SLOT(restartSlot()));
    connect(this, SIGNAL(restartWithoutDepsService()), service, SLOT(restartWithoutDepsSlot()));
    connect(this, SIGNAL(reloadService()), service, SLOT(reloadSlot()));
    connect(this, SIGNAL(destroyService()), service, SLOT(destroySlot()));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(shutdownSlot()));

    service->start(); /* start thread right after initialization */

    /* manage service autostart */
    auto config = new SvdServiceConfig(name);
    if (indicatorFiles->autostart->exists() or config->autoStart) {
        logInfo() << "Performing autostart of service:" << name;
        emit startService();
    }

    if (indicatorFiles->running->exists()) {
        logInfo() << "Cleaning old crontab indicators for service:" << name;
        Q_FOREACH(auto entry, config->schedulerActions) {
            QString indicator = config->prefixDir() + DEFAULT_SERVICE_CRON_WORKING_FILE + "-" + entry->sha;
            logDebug() << "Removing old cron indicator:" << indicator;
            QFile::remove(indicator);
        }
        logInfo() << "Found already started service. Resuming background tasks for service:" << name;
        service->startSlot();
    }

    delete config;
}


void SvdServiceWatcher::shutdownSlot() {
    qDebug() << "Invoked shutdown slot.";
    qDebug() << "Emitting stopService signal.";
    shutdownDefaultVPNNetwork();
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

    /* reconfigure */
    if (triggerFiles->reConfigure->exists()) {
        triggerFiles->reConfigure->remove();
        logDebug() << "Emitting reConfigureService() signal.";
        emit reConfigureService();
        return;
    }

    /* reconfigure without dependencies*/
    if (triggerFiles->reConfigureWithoutDeps->exists()) {
        triggerFiles->reConfigureWithoutDeps->remove();
        logDebug() << "Emitting reConfigureWithoutDepsService() signal.";
        emit reConfigureWithoutDepsService();
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

    /* startWithoutDeps */
    if (triggerFiles->startWithoutDeps->exists()) {
        triggerFiles->startWithoutDeps->remove();
        if (indicatorFiles->running->exists())
            logWarn() << "Interrupted emission of startWithoutDepsService() signal. Service is already running.";
        else {
            logDebug() << "Emitting startWithoutDepsService() signal.";
            emit startWithoutDepsService();
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

    /* stopWithoutDeps */
    if (triggerFiles->stopWithoutDeps->exists()) {
        triggerFiles->stopWithoutDeps->remove();
        if (indicatorFiles->running->exists()) {
            logDebug() << "Emitting stopWithoutDepsService() signal.";
            emit stopWithoutDepsService();
        } else
            logWarn() << "Interrupted emission of stopWithoutDepsService() signal. Service is not running.";
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

    /* restartWithoutDeps */
    if (triggerFiles->restartWithoutDeps->exists()) {
        triggerFiles->restartWithoutDeps->remove();
        logDebug() << "Emitting restartWithoutDepsService() signal.";
        emit restartWithoutDepsService();
        return;
    }

    /* Check if directory wasn't removed */
    if (not QFile::exists(dir)) {
        logWarn() << "Exploding service watcher and whole family of:" << appName;
        emit destroyService();
    }

}


void SvdServiceWatcher::fileChangedSlot(const QString& file) {
    logDebug() << "File changed:" << file;

}


SvdServiceWatcher::~SvdServiceWatcher() {
    delete triggerFiles;
    delete indicatorFiles;
    if (fileEvents) {
        fileEvents->unregisterFile(dataDir);
    }
    this->deleteLater();
}
