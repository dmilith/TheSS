/**
 *  @author dmilith, tallica
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "service.h"
#include "process.h"


SvdService::SvdService(const QString& name) {
    /* setup service */
    this->name = name;
    this->uptime = new QElapsedTimer();
    logTrace() << "Creating SvdService with name" << this->name;

    /* setup baby sitter */
    babySitter = new QTimer(this);
    connect(babySitter, SIGNAL(timeout()), this, SLOT(babySitterSlot()));
    babySitter->start(BABYSITTER_TIMEOUT_INTERVAL / 1000); // miliseconds
}


SvdService::~SvdService() {
    logInfo() << "Service had uptime:" << toHMS(getUptime());
    delete uptime;
    delete babySitter;
}


qint64 SvdService::getUptime() {
    return uptime->elapsed() / 1000; // seconds
}


/* baby sitting slot is used to watch service pid */
void SvdService::babySitterSlot() {
    auto config = new SvdServiceConfig(name);

    /* look for three required files as indicators of already running services software */
    bool filesExistance = QFile::exists(config->prefixDir() + "/.domain") && QFile::exists(config->prefixDir() + "/.ports") && QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
    if (not filesExistance) {
        logDebug() << "Skipping babysitter spawn for service:" << name << ", because no service baby around.";
        delete config;
        return;
    } else
        logDebug() << "Babysitter invoked for:" << name;
    QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
    validateSlot();

    if (config->alwaysOn) {

        if (config->babySitter->commands.isEmpty()) {
            logTrace() << "Dealing with default service baby sitter for" << name;

            /* checking status of pid of service */
            if (QFile::exists(servicePidFile)) {
                bool ok;
                logDebug() << "Babysitter has found service pid for" << name;
                QString aPid = QString(readFileContents(servicePidFile).c_str()).trimmed();
                uint pid = aPid.toInt(&ok, 10);
                logDebug() << "Checking status of pid:" << QString::number(pid);
                if (ok) {
                    if (pidIsAlive(pid)) {
                        logDebug() << "Service:" << name << "seems to be alive and kicking.";
                    } else {
                        logError() << "Service:" << name << "seems to be down. Performing restart.";
                        restartSlot();
                    }
                } else {
                    logWarn() << "Pid file is damaged or doesn't contains valid pid. File will be removed:" << servicePidFile;
                    QFile::remove(servicePidFile);
                }

            } else {
                logDebug() << "No service pid file found for service:" << name << "Ignoring this problem (might be auto pid managment defined in software)";
                // restartSlot();
            }

            /* perform additional port check if watchPort property is set to true */
            if (config->watchPort) {
                logDebug() << "Checking port availability for service" << name;

                /* check static port if it's defined for service */
                if (config->staticPort != -1) {
                    int port = registerFreeTcpPort(config->staticPort);
                    if (port == config->staticPort) {
                        /* if port is equal then it implies that nothing is listening on that port */
                        logError() << "Babysitter has found unoccupied static port:" << config->staticPort << "registered for service" << name;
                        restartSlot();
                    }

                /* check dynamic port for service */
                } else {
                    QString portFilePath = config->prefixDir() + QString(DEFAULT_SERVICE_PORTS_FILE);
                    if (QFile::exists(portFilePath)) {
                        int currentPort = QString(readFileContents(portFilePath).c_str()).trimmed().toInt();
                        int port = registerFreeTcpPort(currentPort);
                        logDebug() << "Port compare:" << currentPort << "with" << port << "(should be different)";
                        if (port == currentPort) {
                            /* if port is equal then it implies that nothing is listening on that port */
                            logError() << "Babysitter has found unoccupied dynamic port:" << currentPort << "registered for service" << name;
                            restartSlot();
                        }
                    } else {
                        logError() << "Babysitter hasn't found port file for service" << name;
                    }
                }
            }

        /* case when custom babysitter must be invoked, cause there's f.e. auto managment of pid by service */
        } else {
            logTrace() << "Dealing with custom service baby sitter for" << name << "with commands:" << config->babySitter->commands;

            auto babySit = new SvdProcess(name);
            babySit->spawnProcess(config->babySitter->commands);
            babySit->waitForFinished(-1); // TODO: implement support for config->babySitter->expectOutputTimeout
            babySit->kill();
            logDebug() << "Checking contents of file:" << babySit->outputFile;
            if (not expect(readFileContents(babySit->outputFile).c_str(), config->babySitter->expectOutput)) {
                logError() << "Failed expectations of service:" << name << "with expected output of babySitter slot:" << config->babySitter->expectOutput;
                writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "BabySitter expectations failed in:" + babySit->outputFile +  " - No match for: '" + config->babySitter->expectOutput + "'");
            } else {
                logDebug() << "Babysitter expectations passed for service:" << name;
            }
            delete babySit;

        }
    } else {
        logTrace() << "alwaysOn option disabled for service:" << name;
    }
    delete config;
}


/* install software */
void SvdService::installSlot() {
    logDebug() << "Invoked install slot for service:" << name;

    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE;
    if (config->serviceInstalled()) {
        logInfo() << "No need to install service" << name << "because it's already installed.";
    } else {
        logDebug() << "Loading service igniter" << name;

        logTrace() << "Launching commands:" << config->install->commands;
        auto proc = new SvdProcess(name);
        touch(indicator);
        proc->spawnProcess(config->install->commands);
        proc->waitForFinished(-1); // no timeout
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->install->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of install slot:" << config->install->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->install->expectOutput + "'");
        }

        /* inform output about some kind of a problem */
        if (config->serviceInstalled()) {
            logDebug() << "Found installed file indicator of software:" << config->softwareName << ", which is base for service:" << name;
        } else { /* software wasn't installed, generate error */
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Installation failed for service:" + config->name);
        }
        QFile::remove(indicator); // this indicates finish of installing process

        logTrace() << "After proc install execution:" << name;
        delete proc;
    }
    delete config;
}


void SvdService::configureSlot() {
    logDebug() << "Invoked configure slot for service:" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to configure service" << name << "because it's already configuring.";
    } else {
        logTrace() << "Loading service igniter" << name;
        touch(indicator);
        logTrace() << "Launching commands:" << config->configure->commands;
        auto proc = new SvdProcess(name);
        proc->spawnProcess(config->configure->commands);
        proc->waitForFinished(-1);
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->configure->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of configure slot:" << config->configure->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->configure->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After proc configure execution:" << name;
        delete proc;
    }
    delete config;
}


void SvdService::startSlot() {
    logDebug() << "Invoked start slot for service:" << name;
    uptime->start();

    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to run service" << name << "because it's already running.";
    } else {
        if (!config->serviceInstalled()) {
            logInfo() << "Service" << name << "isn't yet installed. Proceeding with installation.";
            installSlot();
            logInfo() << "Service" << name << "isn't yet configured. Proceeding with configuration.";
            configureSlot();
        }
        logInfo() << "Validating service" << name;
        validateSlot(); // invoke validation before each startSlot

        logInfo() << "Launching service" << name;
        logTrace() << "Launching commands:" << config->start->commands;
        auto proc = new SvdProcess(name);
        proc->spawnProcess(config->start->commands);

        touch(indicator);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE);

        proc->waitForFinished(-1);
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->start->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of start slot:" << config->start->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->start->expectOutput + "'");
        }

        logTrace() << "After proc start execution:" << name;
        delete proc;
    }
    delete config;
}


void SvdService::afterStartSlot() {
    logDebug() << "Invoked after start slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to afterStart service" << name << "because it's already afterStarting.";
    } else {
        logTrace() << "Launching commands:" << config->afterStart->commands;
        touch(indicator);
        auto proc = new SvdProcess(name);
        proc->spawnProcess(config->afterStart->commands);
        proc->waitForFinished(-1);
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->afterStart->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of afterStart slot:" << config->afterStart->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->afterStart->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After proc afterStart execution:" << name;
        delete proc;
    }
    delete config;
}


void SvdService::stopSlot() {
    logDebug() << "Invoked stop slot for service:" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
    if (not QFile::exists(indicator)) {
        logInfo() << "No need to stop service" << name << "because it's already stopped.";
    } else {
        auto proc = new SvdProcess(name);
        logInfo() << "Stopping service" << name << "after" << toHMS(getUptime()) << "seconds of uptime.";
        delete uptime;
        uptime = new QElapsedTimer(); // reset uptime count

        logTrace() << "Loading service igniter" << name;
        proc->spawnProcess(config->stop->commands); // invoke igniter stop, and then try to look for service.pid in prefix directory:

        QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
        if (QFile::exists(servicePidFile)) {
            uint pid = QString(readFileContents(servicePidFile).c_str()).toUInt();
            logDebug() << "Service pid found:" << QString::number(pid) << "in file:" << servicePidFile;
            kill(pid, SIGTERM);
            QFile::remove(servicePidFile);
            logDebug() << "Service terminated.";
        }
        proc->waitForFinished(-1);
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->stop->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of stop slot:" << config->stop->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->stop->expectOutput + "'");
        }

        QFile::remove(indicator);

        /* remove any other states on stop in case of any kinds of failure /killed ss */
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE);

        logTrace() << "After proc stop execution:" << name;
        delete proc;
    }

    logDebug() << "Stopping internal baby sitter timer for process:" << name;
    if (babySitter->isActive())
        babySitter->stop();
    // delete babySitter;
    delete config;
}


void SvdService::afterStopSlot() {
    logDebug() << "Invoked after stop slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to afterStop service" << name << "because it's already afterStopping.";
    } else {
        touch(indicator);
        logTrace() << "Launching commands:" << config->afterStop->commands;
        auto proc = new SvdProcess(name);
        proc->spawnProcess(config->afterStop->commands);
        proc->waitForFinished(-1);
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->afterStop->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of afterStop slot:" << config->afterStop->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->afterStop->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After proc afterStop execution:" << name;
        delete proc;
    }
    delete config;
}


void SvdService::restartSlot() {
    logDebug() << "Invoked restart slot for service:" << name;
    usleep(DEFAULT_SERVICE_PAUSE_INTERVAL);
    logWarn() << "Restarting service:" << name;
    validateSlot();
    stopSlot();
    startSlot();
    logWarn() << "Service restarted:" << name;
}


void SvdService::reloadSlot() {
    logDebug() << "Invoked reload slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to reload service" << name << "because it's already reloading.";
    } else {
        logTrace() << "Launching commands:" << config->reload->commands;
        touch(indicator);
        auto proc = new SvdProcess(name);
        proc->spawnProcess(config->reload->commands);
        proc->waitForFinished(-1);
        proc->kill();
        if (not expect(readFileContents(proc->outputFile).c_str(), config->reload->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of reload slot:" << config->reload->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->reload->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After proc reload execution:" << name;
        delete proc;
    }
    delete config;
}


void SvdService::validateSlot() {
    logDebug() << "Invoked validate slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);

    /* don't perform validation if no validate hook is defined */
    if (not config->validate->commands.isEmpty()) {
        QString indicator = config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE;
        if (QFile::exists(indicator)) {
            logInfo() << "No need to validate service" << name << "because it's already validating.";
        } else {
            logTrace() << "Launching commands:" << config->validate->commands;
            touch(indicator);
            auto proc = new SvdProcess(name);
            proc->spawnProcess(config->validate->commands);
            proc->waitForFinished(-1);
            proc->kill();
            if (not expect(readFileContents(proc->outputFile).c_str(), config->validate->expectOutput)) {
                logError() << "Failed expectations of service:" << name << "with expected output of validate slot:" << config->validate->expectOutput;
                writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + proc->outputFile +  " - No match for: '" + config->validate->expectOutput + "'");
            }

            QFile::remove(indicator);
            logTrace() << "After proc validate execution:" << name;
            delete proc;
        }
    } else {
        logDebug() << "Skipping validate slot (not defined in igniter) for service:" << name;
    }
    delete config;
}
