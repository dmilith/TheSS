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
}


/* thread */
void SvdService::run() {
    /* first init of uptime timer */
    this->uptime = new QElapsedTimer();
    logTrace() << "Creating SvdService with name" << this->name;

    /* setup baby sitter */
    babySitter = new QTimer(this);
    connect(babySitter, SIGNAL(timeout()), this, SLOT(babySitterSlot()));
    babySitter->start(BABYSITTER_TIMEOUT_INTERVAL / 1000); // miliseconds

    exec();
}


SvdService::~SvdService() {
    logInfo() << "Service had uptime:" << toHMS(getUptime());
    delete uptime;
    delete babySitter;
}


qint64 SvdService::getUptime() {
    return uptime->elapsed() / 1000; // seconds
}


bool SvdService::checkProcessStatus(pid_t pid) {
    logDebug() << "Checking status of pid:" << QString::number(pid);
    logInfo() << "Performing additional process internal OS status check:";

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
                    logFatal() << "Zombie state found for process with name:" << name << "and pid:" << pid;
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
                logFatal() << "Zombie state found for process with name:" << name << "and pid:" << pid;
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

    /* look for three required files as indicators of already running services software */
    bool filesExistance = QFile::exists(config->prefixDir() + "/.domain") && QFile::exists(config->prefixDir() + "/.ports") && QFile::exists(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
    if (not filesExistance) {
        logDebug() << "Skipping babysitter spawn for service:" << name << ", because no service baby around.";
        delete config;
        return;
    } else
        logDebug() << "Babysitter invoked for:" << name;
    QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
    emit validateSlot();

    if (config->alwaysOn) {

        if (config->babySitter->commands.isEmpty()) {
            logTrace() << "Dealing with default service baby sitter for" << name;

            /* checking status of pid of service */
            if (QFile::exists(servicePidFile)) {
                bool ok;
                logDebug() << "Babysitter has found service pid for" << name;
                QString aPid = QString(readFileContents(servicePidFile).c_str()).trimmed();
                pid_t pid = aPid.toInt(&ok, 10);

                if (ok) {
                    if (pidIsAlive(pid)) {
                        if (checkProcessStatus(pid)) {
                            logDebug() << "Service:" << name << "seems to be alive and kicking.";
                        } else {
                            logFatal() << "Something is wrong with system status of service:" << name << "It will be restarted";
                            emit restartSlot();
                        }
                    } else {
                        logError() << "Service:" << name << "seems to be down. Performing restart.";
                        emit restartSlot();
                    }
                } else {
                    logWarn() << "Pid file is damaged or doesn't contains valid pid. File will be removed:" << servicePidFile;
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
                        logError() << "Babysitter has found unoccupied static port:" << config->staticPort << "registered for service" << name;
                        emit restartSlot();
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
                            emit restartSlot();
                        }
                    } else {
                        logFatal() << "Babysitter hasn't found port file for service" << name;
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
            if (not expect(readFileContents(babySit->outputFile).c_str(), config->babySitter->expectOutput)) {
                logError() << "Failed expectations of service:" << name << "with expected output of babySitter slot:" << config->babySitter->expectOutput;
                writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "BabySitter expectations failed in:" + babySit->outputFile +  " - No match for: '" + config->babySitter->expectOutput + "'");
                emit restartSlot();
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
        auto process = new SvdProcess(name);
        touch(indicator);
        process->spawnProcess(config->install->commands);
        process->waitForFinished(-1); // no timeout
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->install->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of install slot:" << config->install->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->install->expectOutput + "'");
        }

        /* inform output about some kind of a problem */
        if (config->serviceInstalled()) {
            logDebug() << "Found installed file indicator of software:" << config->softwareName << ", which is base for service:" << name;
        } else { /* software wasn't installed, generate error */
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Installation failed for service:" + config->name);
        }
        QFile::remove(indicator); // this indicates finish of installing process

        logTrace() << "After proc install execution:" << name;
        delete process;
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->configure->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->configure->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of configure slot:" << config->configure->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->configure->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After process configure execution:" << name;
        delete process;
    }
    delete config;
}


void SvdService::startSlot() {
    logDebug() << "Invoked start slot for service:" << name;
    this->uptime = new QElapsedTimer();
    uptime->start();

    logTrace() << "Loading service igniter" << name;
    auto config = new SvdServiceConfig(name);
    QString indicator = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
    if (QFile::exists(indicator)) {
        logInfo() << "No need to run service" << name << "because it's already running.";
    } else {
        if (!config->serviceInstalled()) {
            logInfo() << "Service" << name << "isn't yet installed. Proceeding with installation.";
            emit installSlot();
            logInfo() << "Service" << name << "isn't yet configured. Proceeding with configuration.";
            emit configureSlot();
        }
        logInfo() << "Validating service" << name;
        emit validateSlot(); // invoke validation before each startSlot

        logInfo() << "Launching service" << name;
        logTrace() << "Launching commands:" << config->start->commands;
        auto process = new SvdProcess(name);
        process->spawnProcess(config->start->commands);

        touch(indicator);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE);

        if (not babySitter->isActive())
            babySitter->start();

        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->start->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of start slot:" << config->start->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->start->expectOutput + "'");
        }

        logTrace() << "After process start execution:" << name;
        delete process;
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->afterStart->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->afterStart->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of afterStart slot:" << config->afterStart->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->afterStart->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After process afterStart execution:" << name;
        delete process;
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
        auto process = new SvdProcess(name);
        logInfo() << "Stopping service" << name << "after" << toHMS(getUptime()) << "seconds of uptime.";
        delete uptime;
        uptime = new QElapsedTimer(); // reset uptime count

        logTrace() << "Loading service igniter" << name;
        process->spawnProcess(config->stop->commands); // invoke igniter stop, and then try to look for service.pid in prefix directory:

        QString servicePidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
        if (QFile::exists(servicePidFile)) {
            uint pid = QString(readFileContents(servicePidFile).c_str()).toUInt();
            logDebug() << "Service pid found:" << QString::number(pid) << "in file:" << servicePidFile;
            deathWatch(process->pid());
            kill(pid, SIGTERM);
            QFile::remove(servicePidFile);
            logDebug() << "Service terminated.";
        }
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->stop->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of stop slot:" << config->stop->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->stop->expectOutput + "'");
        }

        QFile::remove(indicator);

        /* remove any other states on stop in case of any kinds of failure /killed ss */
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE);

        logTrace() << "After process stop execution:" << name;
        delete process;
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->afterStop->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->afterStop->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of afterStop slot:" << config->afterStop->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->afterStop->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After process afterStop execution:" << name;
        delete process;
    }
    delete config;
}


void SvdService::restartSlot() {
    logDebug() << "Invoked restart slot for service:" << name;
    usleep(DEFAULT_SERVICE_PAUSE_INTERVAL);
    logWarn() << "Restarting service:" << name;
    emit validateSlot();
    emit stopSlot();
    emit startSlot();
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
        auto process = new SvdProcess(name);
        process->spawnProcess(config->reload->commands);
        process->waitForFinished(-1);
        deathWatch(process->pid());
        if (not expect(readFileContents(process->outputFile).c_str(), config->reload->expectOutput)) {
            logError() << "Failed expectations of service:" << name << "with expected output of reload slot:" << config->reload->expectOutput;
            writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->reload->expectOutput + "'");
        }

        QFile::remove(indicator);
        logTrace() << "After process reload execution:" << name;
        delete process;
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
            auto process = new SvdProcess(name);
            process->spawnProcess(config->validate->commands);
            process->waitForFinished(-1);
            deathWatch(process->pid());
            if (not expect(readFileContents(process->outputFile).c_str(), config->validate->expectOutput)) {
                logError() << "Failed expectations of service:" << name << "with expected output of validate slot:" << config->validate->expectOutput;
                writeToFile(config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE, "Expectations Failed in:" + process->outputFile +  " - No match for: '" + config->validate->expectOutput + "'");
            }

            QFile::remove(indicator);
            logTrace() << "After process validate execution:" << name;
            delete process;
        }
    } else {
        logDebug() << "Skipping validate slot (not defined in igniter) for service:" << name;
    }
    delete config;
}
