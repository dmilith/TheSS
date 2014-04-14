/**
 *  @author dmilith, tallica
 *
 *   © 2013-2014 - VerKnowSys
 *
 */


#include "service.h"
#include "process.h"


void SvdService::setupDefaultVPNNetwork() {
    #ifdef __FreeBSD__
        logInfo() << "Launching VPN Network Setup";
        auto aProc = new SvdProcess("VPN-setup", 0, false); // don't redirect output
        aProc->spawnProcess(DEFAULT_VPN_INTERFACE_SETUP_COMMAND, DEFAULT_VPN_SPAWN_SHELL);
        aProc->waitForFinished(DEFAULT_PROCESS_TIMEOUT);
        aProc->deleteLater();
    #endif
}


void SvdService::shutdownDefaultVPNNetwork() {
    #ifdef __FreeBSD__
        logInfo() << "Shutting down VPN Network Setup";
        auto aProc = new SvdProcess("VPN-setup", 0, false); // don't redirect output
        aProc->spawnProcess(DEFAULT_VPN_INTERFACE_SHUTDOWN_COMMAND, DEFAULT_VPN_SPAWN_SHELL);
        aProc->waitForFinished(DEFAULT_PROCESS_TIMEOUT);
        aProc->deleteLater();
    #endif
}


void SvdService::loadServiceConfig(const QString& nme) {
    auto testConf = new SvdServiceConfig(nme);
    if (testConf->valid()) {
        if (config)
            config->deleteLater();

        config = new SvdServiceConfig(nme);
        if (configCache)
            configCache->deleteLater();

        configCache = new SvdServiceConfig(nme);
    } else { /* use cached values only in case of validation error */
        if (configCache == NULL) {
            if (config)
                config->deleteLater();

            config = new SvdServiceConfig(); /* fill with default config */
        } else
            config = configCache;
    }
    testConf->deleteLater();
}


void SvdService::notificationSend(const QString& notificationMessage, NotificationLevels level) {
    if (level >= config->notificationLevel) {
    loadServiceConfig(name);
        logDebug() << "Allowed notification with level:" << QString::number(level) << "vs" << QString::number(config->notificationLevel);
        notification(notificationMessage, level);
    } else {
        logDebug() << "Discarded notification with level:" << QString::number(level) << "vs" << QString::number(config->notificationLevel);
    }
}


void SvdService::notificationSend(const QString& notificationMessage) {
    notificationSend(notificationMessage, NOTIFY);
}


SvdService::SvdService(const QString& name) {
    /* setup service */

    this->name = name;
    this->hash = createHash(name);
    this->dependencyServices = QList<SvdService*>();
    this->uptime.invalidate();
    this->networkManager = new QNetworkAccessManager(this);
    loadServiceConfig(name);
}


const QString SvdService::createHash(const QString& nme) {
    /* generate unique sha1 hash from date */
    auto sha1Hash = new QCryptographicHash(QCryptographicHash::Sha1);
    auto content = nme + QDateTime::currentDateTime().toString();
    sha1Hash->addData(content.toUtf8(), content.length());
    auto aHash = sha1Hash->result().toHex();
    delete sha1Hash;
    return aHash;
}


/* thread */
void SvdService::run() {

    /* setup baby sitter */
    babySitter.setInterval(BABYSITTER_TIMEOUT_INTERVAL / 1000); // miliseconds
    connect(&babySitter, SIGNAL(timeout()), this, SLOT(babySitterSlot()));

    /* setup cron sitter */
    cronSitter.setInterval(DEFAULT_CRON_CHECK_DELAY / 1000);
    connect(&cronSitter, SIGNAL(timeout()), this, SLOT(cronSitterSlot()));

    /* setup http watches */
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));

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


void SvdService::finishedSlot(QNetworkReply* aReply) { /* network check slot */
    if (aReply->error() == QNetworkReply::NoError) {
        logDebug() << "Http check passed for url:" << aReply->url();
    } else {
        logError() << "Failed to contact one of http url:" << aReply->url() << "! Error:" << aReply->errorString();
        notificationSend("Http url of service: " + name + ", failed to respond: " + aReply->url().toString() + ". Service will be restarted.", ERROR);
        emit restartSlot();
    }
    aReply->deleteLater();
}


const QString SvdService::releasePostfix() {
    loadServiceConfig(name);
    QString rdir = DEFAULT_RELEASES_DIR + config->releaseName();
    return rdir;
}


/* baby sitting slot is used to watch service pid */
void SvdService::babySitterSlot() {
    loadServiceConfig(name);
    if ( /* check all state files */
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_DEPLOYING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE) or
        QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE)
    ) {
        logInfo() << "Skipping babysitter, service:" << name << "is still busy.";
        return;
    }
    if (not QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE)) {
        logDebug() << "Skipping babysitter, service is not running:" << name;
        stopSitters();
        exit();
        return;
    }
    /* look for three required files as indicators of already running services software */
    bool filesExistance = QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
    if (not filesExistance) {
        logDebug() << "Skipping babysitter spawn for service:" << name;
        return;
    } else
        logDebug() << "Babysitter invoked for:" << name;
    QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PIDS_DIR + config->releaseName() + DEFAULT_SERVICE_PID_FILE;

    /* support separated http url check: */
    if (config->watchHttpAddresses.isEmpty()) {
        logDebug() << "No urls to watch for service" << name;
    } else {
        Q_FOREACH(QString anUrl, config->watchHttpAddresses) {
            logDebug() << "Performing http check of http url:" << anUrl << "for service:" << name;
            QUrl url(anUrl);
            networkManager->get(QNetworkRequest(url));
        }
    }


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
                            notificationSend(msg, ERROR);
                            emit restartSlot();
                            return;
                        }
                    } else {
                        QString msg = "Service: " + name + " seems to be down. Performing restart.";
                        notificationSend(msg, ERROR);
                        emit restartSlot();
                        return;
                    }
                } else {
                    QString msg = "Pid file is damaged or doesn't contains valid pid. File will be removed: " + servicePidFile;
                    notificationSend(msg, ERROR);
                    QFile::remove(servicePidFile);
                    emit restartSlot();
                    return;
                }

            } else {
                logDebug() << "No service pid file found for service:" << name << "Ignoring this problem (might be auto pid managment defined in software)";
            }

            if (config->watchSocket) {
                auto socketFile = getServiceDataDir(name) + DEFAULT_SERVICE_SOCKET_FILE;
                logDebug() << "Checking UNIX socket availability (" << socketFile << ") for service:" << name;
                if (not QFile::exists(socketFile)) {
                    QString msg = "Socket file not found for service: " + name + ". Restarting service.";
                    notificationSend(msg, ERROR);
                    emit restartSlot();
                    return;

                } else
                    logDebug() << "Socket exists. Service seems to be just fine.";
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
                        notificationSend(msg, ERROR);
                        emit restartSlot();
                        return;
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
                            // if (config->webApp) {
                            //     QString msg = "Babysitter has found unoccupied web-app port: " + QString::number(currentPort) + " registered for service: " + name + ". Emitting startSlot just in case";
                            //     notificationSend(msg, NOTIFY);
                            //     QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
                            //     emit startWithoutDepsSlot();
                            //     return;
                            // } else {
                                QString msg = "Babysitter has found unoccupied dynamic port: " + QString::number(currentPort) + " registered for service: " + name;
                                notificationSend(msg, ERROR);
                                emit restartSlot();
                                return;
                            // }
                        }
                    } else {
                        QString msg = "Babysitter hasn't found port file for service: " + name;
                        notificationSend(msg, ERROR);
                        return;
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
                        notificationSend(msg, ERROR);
                        emit restartSlot();
                        return;
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
                            notificationSend(msg, ERROR);
                            emit restartSlot();
                            return;
                        }
                    } else {
                        QString msg = "Babysitter hasn't found port file for service: " + name;
                        notificationSend(msg, ERROR);
                        return;
                    }
                }

            }

        /* case when custom babysitter must be invoked, cause there's f.e. auto managment of pid by service */
        } else {
            logTrace() << "Dealing with custom service baby sitter for" << name << "with commands:" << config->babySitter->commands;

            auto babySit = new SvdProcess(name);
            babySit->spawnProcess(config->babySitter->commands, config->shell);
            babySit->waitForFinished(DEFAULT_PROCESS_TIMEOUT); // TODO: implement support for config->babySitter->expectOutputTimeout
            // deathWatch(babySit->pid());
            logDebug() << "Checking contents of file:" << babySit->outputFile;
            if (not expect(readFileContents(babySit->outputFile), config->babySitter->expectOutput)) {
                QString msg = name + " failed in babySitter slot: " + config->babySitter->expectOutput;
                notificationSend(msg, ERROR);
                emit restartSlot();
                babySit->deleteLater();
                return;
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
}


/* install software */
void SvdService::installSlot() {
    loadServiceConfig(name);
    logDebug() << "Invoked install slot for service:" << name;
    getOrCreateDir(config->prefixDir());
    if (config->webApp)
        getOrCreateDir(config->prefixDir() + releasePostfix());

    QString indicator = config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE;
    if (config->serviceInstalled()) {
        logInfo() << "No need to install already installed service:" << name;
    } else {
        logDebug() << "Loaded service igniter" << name;
        logTrace() << "Launching commands:" << config->install->commands;
        auto process = new SvdProcess(name);
        touch(indicator);
        process->spawnProcess(config->install->commands, config->shell);
        process->waitForFinished(DEFAULT_INSTALL_TIMEOUT);
        // deathWatch(process->pid());
        QFile::remove(indicator); // this indicates finish of installing process
        if (not expect(readFileContents(process->outputFile), config->install->expectOutput)) {
            QString msg = name + " failed in install slot: " + config->babySitter->expectOutput;
            notificationSend(msg, ERROR);
        }

        /* inform output about some kind of a problem */
        // if (config->serviceInstalled()) {
        //     logDebug() << "Found installed file indicator of software:" << config->softwareName << ", which is base for service:" << name;
        // } else { /* software wasn't installed, generate error */
        //     QString msg = "Installation failed for service: " + name;
        //     notificationSend(msg, ERROR);
        // }

        logTrace() << "After proc install execution:" << name;
        process->deleteLater();
    }
}


void SvdService::reConfigureSlot(bool withDeps) {
    loadServiceConfig(name);
    notificationSend("Performing reconfiguration and restart of service: " + name);
    QString configuredIndicator = config->prefixDir() + DEFAULT_SERVICE_CONFS_DIR + config->releaseName() + DEFAULT_SERVICE_CONFIGURED_FILE;
    QFile::remove(configuredIndicator);
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
    loadServiceConfig(name);
    logDebug() << "Invoked configure slot for service:" << name;
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE;
    QString configuredIndicator = config->prefixDir() + DEFAULT_SERVICE_CONFS_DIR + config->releaseName() + DEFAULT_SERVICE_CONFIGURED_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to configure service" << name << "because it's already configuring.";
    } else if (QFile::exists(configuredIndicator)) {
        notificationSend("Service already configured: " + name);
    } else {
        logInfo() << "Configuring service:" << name;
        touch(indicator);
        logTrace() << "Launching commands:" << config->configure->commands;
        auto process = new SvdProcess(name);
        process->spawnProcess(config->configure->commands, config->shell);
        process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);
        QFile::remove(indicator);
        // deathWatch(process->pid());
        logInfo() << "Service configured:" << name;
        if (not expect(readFileContents(process->outputFile), config->configure->expectOutput)) {
            QString msg = name + " failed in configure hook. exp: '" + config->configure->expectOutput + "'";
            notificationSend(msg, ERROR);
        } else
            touch(config->prefixDir() + DEFAULT_SERVICE_CONFS_DIR + config->releaseName() + DEFAULT_SERVICE_CONFIGURED_FILE);

        logTrace() << "After process configure execution:" << name;
        process->deleteLater();
    }
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
    mtx.lock();
    logTrace() << "Loading service igniter" << name;
    loadServiceConfig(name);
    logDebug() << "Invoked start slot for service:" << name;
    uptime.start();

    auto map = getDiskFree(config->prefixDir());
    Q_FOREACH(QString value, map.keys()) {
        logDebug() << "Free disk space in service directory:" << value << "->" << map[value];
        if (map[value] <= config->minimumRequiredDiskSpace) {
            QString msg = "Insufficient disk space for service: " + config->name + " on domains: " + config->domains.join(", ") + ". Expected disk space amount (MiB): " + QString::number(config->minimumRequiredDiskSpace) + " but disk: " + value + " has only: " + map[value] + "!";
            notificationSend(msg, ERROR);
            mtx.unlock();
            return;
        }
    }

    // auto defaultLogFile = config->prefixDir() + DEFAULT_SERVICE_LOG_FILE;
    // if (QFile::exists(defaultLogFile)) {
    //     logDebug() << "Rotating last log";
    //     rotateFile(defaultLogFile);
    // }
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
            logInfo() << "Service:" << name << "not configured yet! Configuring..";
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
            notificationSend(msg, ERROR);
            /* NOTE: don't try to retry. Notification is enough */
            mtx.unlock();
            return;
        }

        logTrace() << "Launching commands:" << config->start->commands;
        auto process = new SvdProcess(name);
        notificationSend("Launching service: " + name);
        process->spawnProcess(config->start->commands, config->shell);
        process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);

        if (not expect(readFileContents(process->outputFile), config->start->expectOutput)) {
            QString msg = name + " failed in start hook. exp: '" + config->start->expectOutput + "'";
            notificationSend(msg, ERROR);
        } else {
            logInfo() << "Launching cron and baby sitters for service:" << name;

            if (not babySitter.isActive()) {
                logDebug() << "Calling babysitter with standard interval:" << QString::number(BABYSITTER_TIMEOUT_INTERVAL / 1000) << "miliseconds";
                babySitter.start();
            }

            if (config->schedulers.size() > 0) {
                logDebug() << "Initializing cronSitter for" << config->schedulers.size() << "scheduler tasks.";

                logDebug() << "Bounding cron to service:" << name;
                if (not cronSitter.isActive())
                    cronSitter.start();
            }
        }

        process->deleteLater();
    }
    touch(indicator);

    mtx.unlock();

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
    loadServiceConfig(name);
    logTrace() << "Cron sitter slot invoked by:" << name;
    if (not QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE)) {
        if (cronSitter.isActive())
            cronSitter.stop();
        logDebug() << "Skipping cronSitter, service is not running:" << name;
        stopSitters();
        exit();
        return;
    }

    Q_FOREACH(auto entry, config->schedulers) {

        QString indicator = config->prefixDir() + DEFAULT_SERVICE_CRON_WORKING_FILE + "-" + entry->sha;

        auto crontabEntry = new SvdCrontab(entry->cronEntry, entry->commands);
        logTrace() << "Processing crontab entry:" << entry->cronEntry << "with commands:" << entry->commands << "  -=>";
        crontabEntry->pp();

        /* If current time matches cron entry.. */
        if (crontabEntry->cronMatch()) {
            if (QFile::exists(indicator)) {
                logDebug() << "No need to launching cron service with indicator:" << indicator << "because it's already been invoked once for service:" << name;
                delete crontabEntry;
                return;
            }
            logDebug() << "Crontab match! Spawning" << entry->commands;
            touch(indicator); /* in this case it's indicator that it's been invoked once already */

            auto process = new SvdProcess(name);
            process->spawnProcess(entry->commands, config->shell);
            process->waitForStarted(-1);

            if (crontabEntry->isContinuous()) {
                logDebug() << "Cron continuous mode, removing indicator.";
                QFile::remove(indicator);
            }

            /* asynchronous handling of cron jobs: */
            connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater(void)));

        } else {
            QFile::remove(indicator);
        }

        delete crontabEntry;
    }

}


void SvdService::afterStartSlot() {
    loadServiceConfig(name);
    logDebug() << "Invoked after start slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to afterStart service" << name << "because it's already afterStarting.";
    } else {
        logTrace() << "Launching commands:" << config->afterStart->commands;
        touch(indicator);
        auto process = new SvdProcess(name);
        process->spawnProcess(config->afterStart->commands, config->shell);
        process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);
        if (not expect(readFileContents(process->outputFile), config->afterStart->expectOutput)) {
            QString msg = name + " failed in afterStart hook. exp: '" + config->afterStart->expectOutput + "'";
            notificationSend(msg, ERROR);

        }

        QFile::remove(indicator);
        logTrace() << "After process afterStart execution:" << name;

        touch(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
        logInfo() << "Marked service running:" << name;

        process->deleteLater();
    }
}


void SvdService::stopSlot(bool withDeps) {
    mtx.lock();
    loadServiceConfig(name);
    logDebug() << "Invoked stop slot for service:" << name;
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;

    logDebug() << "Stopping internal baby sitter timer for process:" << name;
    stopSitters();

    /* stop main application first, dependencies after it */
    if (not QFile::exists(indicator)) {
        logInfo() << "No need to stop service" << name << "because it's already stopped.";
        mtx.unlock();
        return;
    } else {
        auto process = new SvdProcess(name);
        logInfo() << "Terminating service" << name << "after" << toHMS(getUptime()) << "of uptime.";
        uptime.invalidate();

        logTrace() << "Loading service igniter" << name;
        process->spawnProcess(config->stop->commands, config->shell); // invoke igniter stop, and then try to look for service.pid in prefix directory:
        process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);

        QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PIDS_DIR + config->releaseName() + DEFAULT_SERVICE_PID_FILE;
        if (QFile::exists(servicePidFile)) {
            uint pid = readFileContents(servicePidFile).trimmed().toUInt();
            if (pid != 0) {
                logInfo() << "Service pid found:" << QString::number(pid);
                logDebug() << "\\_in file:" << servicePidFile;
                deathWatch(pid);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
                logDebug() << "Service terminated.";
            } else
                logWarn() << "PID of service:" << name << "is zero. (empty pid file?)";
        }
        // deathWatch(process->pid());
        notificationSend("Terminating service: " + name);
        if (not expect(readFileContents(process->outputFile), config->stop->expectOutput)) {
            QString msg = name + " failed in stop hook. exp: '" + config->stop->expectOutput + "'";
            notificationSend(msg, ERROR);
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

        logInfo() << "Cleaning crontab indicators for service:" << config->name;
        Q_FOREACH(auto entry, config->schedulers) {
            QString indicator = config->prefixDir() + DEFAULT_SERVICE_CRON_WORKING_FILE + "-" + entry->sha;
            logDebug() << "Removing old cron indicator:" << indicator;
            QFile::remove(indicator);
        }

        logTrace() << "After process stop execution:" << name;
        process->deleteLater();
    }

    /* stop dependencies of service after main app */
    if (withDeps) {
        Q_FOREACH(SvdService *depService, this->dependencyServices) {
            if (depService) {
                notificationSend("Terminating " + depService->name + " - dependency of service: " + name);
                logDebug() << "Invoking stop slot of service dependency:" << depService->name << "with uptime:" << toHMS(depService->getUptime());
                depService->stopSlot();
                depService->exit();
            }
        }
    }

    mtx.unlock();

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
    loadServiceConfig(name);
    logDebug() << "Invoked after stop slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to afterStop service" << name << "because it's already afterStopping.";
    } else {
        touch(indicator);
        logTrace() << "Launching commands:" << config->afterStop->commands;
        auto process = new SvdProcess(name);
        process->spawnProcess(config->afterStop->commands, config->shell);
        process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);
        // deathWatch(process->pid());
        QFile::remove(indicator);
        if (not expect(readFileContents(process->outputFile), config->afterStop->expectOutput)) {
            QString msg = name + " failed in afterStop hook. exp: '" + config->afterStop->expectOutput + "'";
            notificationSend(msg, ERROR);
        }
        logTrace() << "After process afterStop execution:" << name;
        process->deleteLater();

        logInfo() << "Removing running indicator for service:" << name;
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
    }
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
    loadServiceConfig(name);
    logInfo() << "Invoked reload slot for service:" << name;
    logTrace() << "Loading service igniter" << name;
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to reload service" << name << "because it's already reloading.";
    } else {
        logTrace() << "Launching commands:" << config->reload->commands;
        touch(indicator);
        auto process = new SvdProcess(name);
        process->spawnProcess(config->reload->commands, config->shell);
        process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);
        // deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile), config->reload->expectOutput)) {
            QString msg = name + " failed in reload hook. exp: '" + config->reload->expectOutput + "'";
            notificationSend(msg, ERROR);
        }

        QFile::remove(indicator);
        logTrace() << "After process reload execution:" << name;
        process->deleteLater();
    }
}


void SvdService::validateSlot() {
    loadServiceConfig(name);
    logDebug() << "Invoked validate slot for service:" << name << "with uptime:" << toHMS(getUptime());
    logTrace() << "Loading service igniter" << name;

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
            process->spawnProcess(config->validate->commands, config->shell);
            process->waitForFinished(DEFAULT_PROCESS_TIMEOUT);

            // deathWatch(process->pid());
            if (not expect(readFileContents(process->outputFile), config->validate->expectOutput)) {
                QString msg = name + " failed in validate hook. exp: '" + config->validate->expectOutput + "'";
                notificationSend(msg, ERROR);
            }

            QFile::remove(indicator);
            logTrace() << "After process validate execution:" << name;
            process->deleteLater();
        }
    } else {
        logDebug() << "Skipping validate slot (not defined in igniter) for service:" << name;
    }
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
            }
        }
    }
    // this->networkManager->deleteLater();
    this->exit();
}


