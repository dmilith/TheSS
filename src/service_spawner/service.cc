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
    this->dependencyServices = QList<SvdService*>();
    this->uptime.invalidate();
}


/* thread */
void SvdService::run() {

    /* setup baby sitter */
    babySitter.setInterval(BABYSITTER_TIMEOUT_INTERVAL / 1000); // miliseconds
    connect(&babySitter, SIGNAL(timeout()), this, SLOT(babySitterSlot()));

    /* setup cron sitter */
    cronSitter.setInterval(DEFAULT_CRON_CHECK_DELAY / 1000);
    connect(&cronSitter, SIGNAL(timeout()), this, SLOT(cronSitterSlot()));

    logTrace() << "Creating SvdService with name" << this->name;
    exec();
}


qint64 SvdService::getUptime() {
    int value = 0;
    if (uptime.isValid())
        value = uptime.elapsed() / 1000; /* seconds */
    return value;
}


bool SvdService::checkProcessStatus(pid_t pid) {
    logDebug() << "Checking status of pid:" << QString::number(pid);
    logTrace() << "Performing additional process internal OS status check:";

    bool ok = true;

    #ifndef __linux__
        int mib[4];
        size_t len;
        struct kinfo_proc kp;
        len = 4; /* Fill out the first three components of the mib */
        sysctlnametomib("kern.proc.pid", mib, &len);
        mib[3] = pid;
        len = sizeof(kp);
        if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1) {
            perror("sysctl");
            ok = false;
        } else if (len > 0) {
            char state;
            #ifdef __FreeBSD__
                state = kp.ki_stat;
                logDebug() << "Process internal STATUS:" << state;
            #elif defined(__APPLE__)
                state = kp.kp_proc.p_stat;
                logDebug() << "Process internal STATUS:" << state;
            #else
                logDebug() << "Unsupported system found? Skipping";
                return ok;
            #endif

            switch (state) {
                case SRUN:
                    logDebug() << "Running state found for process with name:" << name << "and pid:" << pid;
                    break; /* running */
                case SIDL:
                    logDebug() << "Sleeping state found for process with name:" << name << "and pid:" << pid;
                    break; /* sleeping */
                case SSLEEP:
                    logDebug() << "Disk IO state found for process with name:" << name << "and pid:" << pid;
                    break; /* IO/disk sleep */
                case SSTOP:
                    logDebug() << "Stopped state found for process with name:" << name << "and pid:" << pid;
                    break; /* traced or stopped (by signal) */
                case SZOMB:
                    logError() << "Zombie state found for process with name:" << name << "and pid:" << pid;
                    ok = false;
                    break; /* zombie */
                default:
                    logError() << "Ambigous process state for process with name:" << name << "and pid:" << pid;
                    // TODO: consider setting ok = false here, cause it shouldn't happen
                    break;
            }
        }

    #else // linux:
        FILE *pFileHandle = NULL;

        typedef long long int num;
        num x;

        char name[PATH_MAX];
        char tcomm[PATH_MAX];
        char state;

        char procpath[255];
        sprintf(procpath, "/proc/%d/stat", pid);
        pFileHandle = fopen(procpath, "r");

        fscanf(pFileHandle, "%lld", &x); /* pid is first */
        fscanf(pFileHandle, "%s ", &name); /* then string with name */
        fscanf(pFileHandle, "%c ", &state); /* and finally what's interesting - the state */

        logDebug() << "Process internal STATUS:" << state;
        switch (state) {
            case 'R':
                logDebug() << "Running state found for process with name:" << name << "and pid:" << pid;
                break; /* running */
            case 'S':
                logDebug() << "Sleeping state found for process with name:" << name << "and pid:" << pid;
                break; /* sleeping */
            case 'D':
                logDebug() << "Disk IO state found for process with name:" << name << "and pid:" << pid;
                break; /* IO/disk sleep */
            case 'T':
                logDebug() << "Stopped state found for process with name:" << name << "and pid:" << pid;
                break; /* traced or stopped (by signal) */
            case 'Z':
                logError() << "Zombie state found for process with name:" << name << "and pid:" << pid;
                ok = false;
                break; /* zombie */
            default:
                logError() << "Ambigous process state for process with name:" << name << "and pid:" << pid;
                // TODO: consider setting ok = false here, cause it shouldn't happen
                break;
        }
        fclose(pFileHandle);
    #endif

    return ok;
}


/* baby sitting slot is used to watch service pid */
void SvdService::babySitterSlot() {
    auto config = new SvdServiceConfig(name);

    if ( /* check all state files */
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE)
    ) {
        logDebug() << "Skipping babysitter, service is busy:" << name;
        config->deleteLater();
        return;
    }
    if (not QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE)) {
        logDebug() << "Skipping babysitter, service is not running:" << name;
        config->deleteLater();
        stopSitters();
        exit();
        return;
    }
    /* look for three required files as indicators of already running services software */
    bool filesExistance = QFile::exists(config->prefixDir() + "/.domain") && QDir().exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR) && QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
    if (not filesExistance) {
        logDebug() << "Skipping babysitter spawn for service:" << name << ", because no service baby around.";
        config->deleteLater();
        return;
    } else
        logDebug() << "Babysitter invoked for:" << name;
    QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;

    if (config->alwaysOn) {

        if (config->babySitter->commands.isEmpty()) {
            logTrace() << "Dealing with default service baby sitter for" << name;

            /* checking status of pid of service */
            if (QFile::exists(servicePidFile)) {
                bool ok;
                logDebug() << "Babysitter has found service pid for" << name;
                QString aPid = readFileContents(servicePidFile).trimmed();
                pid_t pid = aPid.toInt(&ok, 10);

                if (ok) {
                    if (pidIsAlive(pid)) {
                        if (checkProcessStatus(pid)) {
                            logDebug() << "Service:" << name << "seems to be alive and kicking.";
                        } else {
                            QString msg = "Something is wrong with system status of service: " + name + " It will be restarted";
                            notification(msg, name, ERROR);
                            emit restartSlot();
                        }
                    } else {
                        QString msg = "Service: " + name + " seems to be down. Performing restart.";
                        notification(msg, name, ERROR);
                        emit restartSlot();
                    }
                } else {
                    QString msg = "Pid file is damaged or doesn't contains valid pid. File will be removed: " + servicePidFile;
                    notification(msg, name, ERROR);
                    QFile::remove(servicePidFile);
                    emit restartSlot();
                }

            } else {
                logDebug() << "No service pid file found for service:" << name << "Ignoring this problem (might be auto pid managment defined in software)";
            }

            /* perform additional port check if watchPort property is set to true */
            if (config->watchPort) {
                logDebug() << "Checking port availability for service" << name;

                /* check static port if it's defined for service */
                if (config->staticPort != -1) {
                    int port = registerFreeTcpPort(config->staticPort);
                    if (port == config->staticPort) {
                        /* if port is equal then it implies that nothing is listening on that port */
                        QString msg = "Babysitter has found unoccupied static port: " + QString::number(config->staticPort) + " registered for service " + name;
                        notification(msg, name, ERROR);
                        emit restartSlot();
                    }

                /* check dynamic port for service */
                } else {
                    QString portFilePath = config->prefixDir() + QString(DEFAULT_SERVICE_PORTS_DIR) + QString(DEFAULT_SERVICE_PORT_NUMBER); /* default port */

                    if (QFile::exists(portFilePath)) {
                        int currentPort = readFileContents(portFilePath).trimmed().toInt();
                        int port = registerFreeTcpPort(currentPort);
                        logDebug() << "Port compare:" << currentPort << "with" << port << "(should be different)";
                        if (port == currentPort) {
                            /* if port is equal then it implies that nothing is listening on that port */
                            QString msg = "Babysitter has found unoccupied dynamic port: " + QString::number(currentPort) + " registered for service: " + name;
                            notification(msg, name, ERROR);
                            emit restartSlot();
                        }
                    } else {
                        QString msg = "Babysitter hasn't found port file for service: " + name;
                        notification(msg, name, ERROR);
                    }
                }
            }

            /* support UDP port check: */
            if (config->watchUdpPort) {
                logDebug() << "Checking UDP port availability for service" << name;

                /* check static port if it's defined for service */
                if (config->staticPort != -1) {
                    int port = registerFreeUdpPort(config->staticPort);
                    if (port == config->staticPort) {
                        /* if port is equal then it implies that nothing is listening on that port */
                        QString msg = "Babysitter has found unoccupied static UDP port: " + QString::number(config->staticPort) + " registered for service " + name;
                        notification(msg, name, ERROR);
                        emit restartSlot();
                    }

                /* check dynamic port for service */
                } else {
                    QString portFilePath = config->prefixDir() + QString(DEFAULT_SERVICE_PORTS_DIR) + QString(DEFAULT_SERVICE_PORT_NUMBER); /* default port */

                    if (QFile::exists(portFilePath)) {
                        int currentPort = readFileContents(portFilePath).trimmed().toInt();
                        int port = registerFreeUdpPort(currentPort);
                        logDebug() << "UDP port compare:" << currentPort << "with" << port << "(should be different)";
                        if (port == currentPort) {
                            /* if port is equal then it implies that nothing is listening on that port */
                            QString msg = "Babysitter has found unoccupied dynamic UDP port: " + QString::number(currentPort) + " registered for service: " + name;
                            notification(msg, name, ERROR);
                            emit restartSlot();
                        }
                    } else {
                        QString msg = "Babysitter hasn't found port file for service: " + name;
                        notification(msg, name, ERROR);
                    }
                }

            }

        /* case when custom babysitter must be invoked, cause there's f.e. auto managment of pid by service */
        } else {
            logTrace() << "Dealing with custom service baby sitter for" << name << "with commands:" << config->babySitter->commands;

            auto babySit = new SvdProcess(name);
            babySit->spawnProcess(config->babySitter->commands);
            babySit->waitForFinished(-1); // TODO: implement support for config->babySitter->expectOutputTimeout
            deathWatch(babySit->pid());
            logDebug() << "Checking contents of file:" << babySit->outputFile;
            if (not expect(readFileContents(babySit->outputFile), config->babySitter->expectOutput)) {
                QString msg = "Failed expectations of service: " + name + " with expected output of babySitter slot: " + config->babySitter->expectOutput;
                notification(msg, name, ERROR);
                emit restartSlot();
            } else {
                logDebug() << "Babysitter expectations passed for service:" << name;
            }
            babySit->deleteLater();

        }

        /* look for orphaned dependency services */
        Q_FOREACH(auto dependency, config->dependencies) {
            bool add = true;
            Q_FOREACH(SvdService *orphan, this->dependencyServices) {
                if (orphan->name == dependency) { // XXX: FIXME: BUG: it's a hack because we may have deep tree of dependencies and here only one level of depth is handled. recursive dependencies aren't handled!
                    add = false;
                }
            }
            if (add) {
                logDebug() << "Orphaned service found:" << dependency;
                SvdService *svce = new SvdService(dependency);
                dependencyServices << svce;
                svce->exit();
            }
        }

    } else {
        logTrace() << "alwaysOn option disabled for service:" << name;
    }
    config->deleteLater();
}


/* install software */
void SvdService::installSlot() {
    logDebug() << "Invoked install slot for service:" << name;

    auto config = new SvdServiceConfig(name);
    logInfo() << "Performing sanity dir checks for service:" << name;

    logDebug() << "Checking notification dirs";
    getOrCreateDir(config->prefixDir());
    getOrCreateDir(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR);
    getOrCreateDir(config->prefixDir() + NOTIFICATIONS_DATA_DIR);
    getOrCreateDir(config->prefixDir() + NOTIFICATIONS_HISTORY_DATA_DIR);

    QString portsDirLocation = config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR;
    if (not QDir().exists(portsDirLocation)) {
        if (QFile::exists(portsDirLocation)) {
            logWarn() << "Found legacy .ports file. Automatically removing this file from service:" << name;
            QFile::remove(portsDirLocation);
        } else
            logDebug() << "No legacy ports file.";
    }

    QString indicator = config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE;
    if (config->serviceInstalled()) {
        logInfo() << "No need to install already installed service:" << name;
    } else {
        logDebug() << "Loaded service igniter" << name;
        logTrace() << "Launching commands:" << config->install->commands;
        auto process = new SvdProcess(name);
        touch(indicator);
        process->spawnProcess(config->install->commands);
        process->waitForFinished(-1); // no timeout
        deathWatch(process->pid());
        QFile::remove(indicator); // this indicates finish of installing process
        if (not expect(readFileContents(process->outputFile), config->install->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of install slot: " + config->babySitter->expectOutput;
            notification(msg, name, ERROR);
        }

        /* inform output about some kind of a problem */
        if (config->serviceInstalled()) {
            logDebug() << "Found installed file indicator of software:" << config->softwareName << ", which is base for service:" << name;
        } else { /* software wasn't installed, generate error */
            QString msg = "Installation failed for service: " + name;
            notification(msg, name, ERROR);
        }

        logTrace() << "After proc install execution:" << name;
        process->deleteLater();
    }
    config->deleteLater();
}


void SvdService::reConfigureSlot(bool withDeps) {
    notification("Performing reconfiguration and restart of service: " + name, name, NOTIFY);
    auto config = new SvdServiceConfig(name);
    QString configuredIndicator = config->prefixDir() + "/.configured";
    QFile::remove(configuredIndicator);
    config->deleteLater();
    emit configureSlot();
    emit restartSlot(withDeps);
}


void SvdService::reConfigureSlot() {
    reConfigureSlot(true);
}


void SvdService::reConfigureWithoutDepsSlot() {
    reConfigureSlot(false);
}


void SvdService::configureSlot() {
    logDebug() << "Invoked configure slot for service:" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE;
    QString configuredIndicator = config->prefixDir() + "/.configured";
    if (QFile::exists(indicator)) {
        logInfo() << "No need to configure service" << name << "because it's already configuring.";
    } else if (QFile::exists(configuredIndicator)) {
        notification("Service already configured: " + name, name, NOTIFY);
    } else {
        logTrace() << "Loading service igniter" << name;
        touch(indicator);
        logTrace() << "Launching commands:" << config->configure->commands;
        auto process = new SvdProcess(name);
        process->spawnProcess(config->configure->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        QFile::remove(indicator);
        logDebug() << "Service configured:" << name;
        if (not expect(readFileContents(process->outputFile), config->configure->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of configure slot - No match for: '" + config->configure->expectOutput + "'";
            notification(msg, name, ERROR);
        } else
            touch(config->prefixDir() + "/.configured");

        logTrace() << "After process configure execution:" << name;
        process->deleteLater();
    }
    config->deleteLater();
}


bool dependencyConfigureOrderLessThan(const QString &a, const QString &b) {
    auto aConf = new SvdServiceConfig(a);
    auto bConf = new SvdServiceConfig(b);
    int aOrder = aConf->configureOrder;
    int bOrder = bConf->configureOrder;
    delete aConf;
    delete bConf;
    return aOrder < bOrder;
}


bool dependencyStartOrderLessThan(const QString &a, const QString &b) {
    auto aConf = new SvdServiceConfig(a);
    auto bConf = new SvdServiceConfig(b);
    int aOrder = aConf->startOrder;
    int bOrder = bConf->startOrder;
    delete aConf;
    delete bConf;
    return aOrder < bOrder;
}


void SvdService::startSlot(bool withDeps) {
    logDebug() << "Invoked start slot for service:" << name;
    uptime.start();

    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);

    auto map = getDiskFree(config->prefixDir());
    Q_FOREACH(QString value, map.keys()) {
        logDebug() << "Free disk space in service directory:" << value << "->" << map[value];
        if (map[value] <= config->minimumRequiredDiskSpace) {
            QString msg = "Insufficient disk space for service: " + config->name + " on domain: " + config->domain + " Expected disk space amount (MiB): " + QString::number(config->minimumRequiredDiskSpace) + " but disk: " + value + " has only: " + map[value] + "!";
            notification(msg, name, ERROR);
        }
    }

    auto defaultLogFile = config->prefixDir() + DEFAULT_SERVICE_LOG_FILE;
    if (QFile::exists(defaultLogFile)) {
        logDebug() << "Rotating last log";
        rotateFile(defaultLogFile);
    }
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to run service" << name << "because it's already running.";
        logDebug() << "Checking babysitter state for service:" << name;
        if (not babySitter.isActive())
            babySitter.start();
        logDebug() << "Checking cronSitter state for service:" << name;
        if (not cronSitter.isActive())
            cronSitter.start();
    } else {
        logDebug() << "Emitting install slot for service:" << name;
        emit installSlot();
        if (not config->serviceConfigured()) {
            logInfo() << "Configuring service:" << name;
            emit configureSlot();
        }

        /* configure all dependencies before continue */
        if (not config->dependencies.isEmpty() && withDeps) {
            qSort(config->dependencies.begin(), config->dependencies.end(), dependencyConfigureOrderLessThan);
            logDebug() << "Dependencies sorted for configure:" << config->dependencies;

            Q_FOREACH(auto dependency, config->dependencies) {
                logInfo() << "Installing and configuring dependency:" << dependency;
                auto depConf = new SvdServiceConfig(dependency);

                SvdService *depService = new SvdService(dependency);
                depService->start();
                depService->installSlot();
                if (not depConf->serviceConfigured()) {
                    depService->configureSlot();
                }
                depService->exit();
                depConf->deleteLater();
            }
        }

        /* after successful installation of core app and configuring dependencies, we may proceed */
        if (not config->dependencies.isEmpty() && withDeps) {
            QFile::remove(indicator);
            qSort(config->dependencies.begin(), config->dependencies.end(), dependencyStartOrderLessThan);
            logInfo() << "Found additional igniter dependency(ies) for service:" << name << "list:" << config->dependencies;

            Q_FOREACH(auto dependency, config->dependencies) {
                logDebug() << "Launching dependency:" << dependency;
                auto depConf = new SvdServiceConfig(dependency);

                /* if dependency is already running - skip it */
                if (not QFile::exists(depConf->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE)) {
                    auto depService = new SvdService(dependency);
                    depService->start();
                    depService->startSlot();
                    dependencyServices << depService;
                    logInfo() << "Launched dependency:" << dependency;
                    depService->exit();
                } else {
                    logInfo() << "Already running dependency:" << dependency;
                }

                depConf->deleteLater();
            }
        } else
            logDebug() << "Empty dependency list for service:" << name;

        logInfo() << "Validating service" << name;
        emit validateSlot(); // invoke validation before each startSlot

        if (QFile::exists(config->prefixDir() + DEFAULT_SERVICE_VALIDATION_FAILURE_FILE)) {
            QString msg = "Validation failure in service: " + name + ". Won't start this service. Fix failure and try again.";
            notification(msg, name, ERROR);
            config->deleteLater();
            /* NOTE: don't try to retry. Notification is enough */
            return;
        }

        notification("Launching service: " + name, name, NOTIFY);

        logTrace() << "Launching commands:" << config->start->commands;
        // touch(indicator);
        auto process = new SvdProcess(name);
        process->spawnProcess(config->start->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());

        if (not expect(readFileContents(process->outputFile), config->start->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of start slot - No match for: '" + config->start->expectOutput + "'";
            notification(msg, name, ERROR);
        } else {
            logInfo() << "Service expectations were successful. Launching cron and baby sitters for service:" << name;

            if (not babySitter.isActive()) {
                logDebug() << "Calling babysitter with standard interval:" << QString::number(BABYSITTER_TIMEOUT_INTERVAL / 1000) << "miliseconds";
                babySitter.start();
            }

            if (config->schedulerActions.size() > 0) {
                logDebug() << "Initializing cronSitter for" << config->schedulerActions.size() << "scheduler tasks.";

                logDebug() << "Bounding cron to service:" << name;
                if (not cronSitter.isActive())
                    cronSitter.start();
            }
        }

        process->deleteLater();
    }
    config->deleteLater();

    /* invoke after start slot */
    logTrace() << "After process start execution:" << name;
    emit afterStartSlot();
}


void SvdService::startSlot() {
    startSlot(true);
}


void SvdService::startWithoutDepsSlot() {
    startSlot(false);
}


void SvdService::cronSitterSlot() {
    logTrace() << "Cron sitter slot invoked by:" << name;
    auto config = new SvdServiceConfig(name);
    if (not QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE)) {
        if (cronSitter.isActive())
            cronSitter.stop();
        logDebug() << "Skipping cronSitter, service is not running:" << name;
        config->deleteLater();
        stopSitters();
        exit();
        return;
    }

    Q_FOREACH(auto entry, config->schedulerActions) {

        QString indicator = config->prefixDir() + DEFAULT_SERVICE_CRON_WORKING_FILE + "-" + entry->sha;

        auto crontabEntry = new SvdCrontab(entry->cronEntry, entry->commands);
        logTrace() << "Processing crontab entry:" << entry->cronEntry << "with commands:" << entry->commands << "  -=>";
        crontabEntry->pp();

        /* If current time matches cron entry.. */
        if (crontabEntry->cronMatch()) {
            if (QFile::exists(indicator)) {
                logDebug() << "No need to launching cron service with indicator:" << indicator << "because it's already been invoked once for service:" << name;
                delete crontabEntry;
                config->deleteLater();
                return;
            }
            logDebug() << "Crontab match! Spawning" << entry->commands;
            touch(indicator); /* in this case it's indicator that it's been invoked once already */

            auto process = new SvdProcess(name);
            process->spawnProcess(entry->commands);
            process->waitForStarted(-1);

            /* asynchronous handling of cron jobs: */
            connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater(void)));

        } else {
            QFile::remove(indicator);
        }

        delete crontabEntry;
    }

    config->deleteLater();
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->afterStart->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile), config->afterStart->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of afterStart slot - No match for: '" + config->afterStart->expectOutput + "'";
            notification(msg, name, ERROR);

        }

        QFile::remove(indicator);
        logTrace() << "After process afterStart execution:" << name;
        process->deleteLater();
    }
    config->deleteLater();
}


void SvdService::stopSlot(bool withDeps) {
    logDebug() << "Invoked stop slot for service:" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
    stopSitters();

    /* stop dependency services */
    if (withDeps) {
        Q_FOREACH(SvdService *depService, this->dependencyServices) {
            if (depService) {
                logDebug() << "Invoking stop slot of service dependency:" << depService->name << "with uptime:" << toHMS(depService->getUptime());
                depService->stopSlot();
                depService->exit();
            }
        }
    }

    if (not QFile::exists(indicator)) {
        logInfo() << "No need to stop service" << name << "because it's already stopped.";
    } else {
        auto process = new SvdProcess(name);
        logInfo() << "Stopping service" << name << "after" << toHMS(getUptime()) << "of uptime.";
        uptime.invalidate();

        logTrace() << "Loading service igniter" << name;
        process->spawnProcess(config->stop->commands); // invoke igniter stop, and then try to look for service.pid in prefix directory:

        QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
        if (QFile::exists(servicePidFile)) {
            uint pid = readFileContents(servicePidFile).toUInt();
            logDebug() << "Service pid found:" << QString::number(pid) << "in file:" << servicePidFile;
            deathWatch(pid);
            QFile::remove(servicePidFile);
            logDebug() << "Service terminated.";
        }
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile), config->stop->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of stop slot - No match for: '" + config->stop->expectOutput + "'";
            notification(msg, name, ERROR);
        }

        // /* remove any other states on stop in case of any kinds of failure /killed ss */
        // // QFile::remove(indicator);

        logDebug() << "Performing after stop indicators cleanup.";
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATION_FAILURE_FILE);

        logTrace() << "After process stop execution:" << name;
        process->deleteLater();
    }

    logDebug() << "Stopping internal baby sitter timer for process:" << name;

    config->deleteLater();

    /* invoke after stop slot */
    emit afterStopSlot();
}


void SvdService::stopSlot() {
    stopSlot(true);
}


void SvdService::stopWithoutDepsSlot() {
    stopSlot(false);
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->afterStop->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        QFile::remove(indicator);
        if (not expect(readFileContents(process->outputFile), config->afterStop->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of afterStop slot - No match for: '" + config->afterStop->expectOutput + "'";
            notification(msg, name, ERROR);
        }
        logTrace() << "After process afterStop execution:" << name;
        process->deleteLater();
    }
    config->deleteLater();
}


void SvdService::restartSlot(bool withDeps) {
    if (QFile::exists(getHomeDir() + DEFAULT_SS_SHUTDOWN_HOOK_FILE)) {
        logWarn() << "Ignoring restart slot of service:" << name << "cause shutdown hook was called.";
    } else {
        logDebug() << "Invoked restart slot for service:" << name;
        // usleep(DEFAULT_SERVICE_PAUSE_INTERVAL);
        logWarn() << "Restarting service:" << name;
        emit stopSlot(withDeps);
        emit startSlot(withDeps);
        logInfo() << "Service restarted:" << name;
    }
}


void SvdService::restartSlot() {
    restartSlot(true);
}


void SvdService::restartWithoutDepsSlot() {
    restartSlot(false);
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->reload->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile), config->reload->expectOutput)) {
            QString msg = "Failed expectations of service: " + name + " with expected output of reload slot - No match for: '" + config->reload->expectOutput + "'";
            notification(msg, name, ERROR);
        }

        QFile::remove(indicator);
        logTrace() << "After process reload execution:" << name;
        process->deleteLater();
    }
    config->deleteLater();
}


void SvdService::validateSlot() {
    logDebug() << "Invoked validate slot for service:" << name << "with uptime:" << toHMS(getUptime());
    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);

    /* don't perform validation if no validate hook is defined */
    if (not config->validate->commands.isEmpty()) {
        QString indicator = config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE;
        if (QFile::exists(indicator)) {
            logInfo() << "No need to validate service" << name << "because it's already validating.";
        } else {
            logTrace() << "Launching commands:" << config->validate->commands;
            /* remove failure state */
            QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATION_FAILURE_FILE);
            /* validation state */
            touch(indicator);
            auto process = new SvdProcess(name);
            process->spawnProcess(config->validate->commands);
            process->waitForFinished(-1);

            deathWatch(process->pid());
            if (not expect(readFileContents(process->outputFile), config->validate->expectOutput)) {
                QString msg = "Failed expectations of service: " + name + " with expected output of validate slot - No match for: '" + config->validate->expectOutput + "'";
                notification(msg, name, ERROR);
            }

            QFile::remove(indicator);
            logTrace() << "After process validate execution:" << name;
            process->deleteLater();
        }
    } else {
        logDebug() << "Skipping validate slot (not defined in igniter) for service:" << name;
    }
    config->deleteLater();
}


void SvdService::stopSitters() {
    logDebug() << "Stopping sitters for service:" << name;

    babySitter.stop();
    cronSitter.stop();
}


void SvdService::destroySlot() {
    logDebug() << "Destroying service:" << name;

    if (not dependencyServices.empty()) {
        logDebug() << "Destroying dependency services of service:" << name;
        Q_FOREACH(SvdService *el, dependencyServices) {
            if (el != NULL) {
                logTrace() << "Deleting dependencyService for service:" << name;
                el->exit();
                // el->deleteLater();
            }
        }
    }

    this->exit();
    // this->deleteLater();
}


