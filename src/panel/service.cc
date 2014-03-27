/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "service.h"


void PanelService::panelAssert(bool predicate, const char* where, const char* message) {
    if (predicate) return;
    else {
        endwin();
        cout << "FATAL: Predicate assertion failed with message: " << message << " in: " << where << endl;
        Q_ASSERT_X(predicate, where, message);
    }
}


PanelService::PanelService(Panel * panel, QFileInfo baseDir){
    this->panel = panel;
    this->baseDir = baseDir;
    name = baseDir.baseName();

    /* check if volatile service state is initialized. No rush */
    while (QDir().exists(getServiceDataDir(name) + DEFAULT_SERVICE_PORTS_DIR) and not QFile::exists(getServiceDataDir(name) + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER)) {
        logTrace() << "Waiting for non-volatile service state of service:" << name;
        usleep(1000);
    }

    auto config = new SvdServiceConfig(name);
    basePath = baseDir.absolutePath() + "/" + baseDir.baseName();

    /* security assertions */
    panelAssert(basePath != "/", "PanelService", "basePath can't be '/'");
    panelAssert(not basePath.contains(" "), "PanelService", "basePath can't contain spaces!");
    if (getuid() == 0) {
        panelAssert(basePath.startsWith("/SystemUsers/"), "PanelService", "basePath must be located in: '/SystemUsers/'");
    } else {
        panelAssert(basePath.startsWith(QString(getenv("HOME"))), "PanelService", "basePath must be located in user home directory!");
    }
    this->dir = QDir(basePath);

    this->log = new Tail(this, basePath + DEFAULT_SERVICE_LOGS_DIR + config->releaseName(), DEFAULT_SERVICE_LOG_FILE);
    this->conf = new Tail(this, basePath + DEFAULT_SERVICE_CONFS_DIR + config->releaseName(), DEFAULT_SERVICE_CONF_FILE);
    this->env = new Tail(this, basePath + DEFAULT_SERVICE_ENVS_DIR + config->releaseName(), DEFAULT_SERVICE_ENV_FILE);

    config->deleteLater();
    refresh();
}

void PanelService::refresh(){

    // logDebug() << "new tail";
    // logTail(basePath, "service.log");

    isRunning = QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE);
    bool isAutostarted = QFile::exists(basePath + AUTOSTART_TRIGGER_FILE);
    bool sv = QFile::exists(basePath + DEFAULT_SERVICE_VALIDATING_FILE);
    bool sc = QFile::exists(basePath + DEFAULT_SERVICE_CONFIGURING_FILE) || QFile::exists(basePath + DEFAULT_SERVICE_DEPLOYING_FILE);
    bool si = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE);
    bool se = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE); // XXX: FIXME: no more .errors file!!
    autostart = QFile::exists(basePath + AUTOSTART_TRIGGER_FILE);

    auto config = new SvdServiceConfig(name);

    this->log->releaseUpdate(basePath + DEFAULT_SERVICE_LOGS_DIR + config->releaseName());
    this->conf->releaseUpdate(basePath + DEFAULT_SERVICE_CONFS_DIR + config->releaseName());
    this->env->releaseUpdate(basePath + DEFAULT_SERVICE_ENVS_DIR + config->releaseName());

    pid = readFileContents(basePath + DEFAULT_SERVICE_PIDS_DIR + config->releaseName() + DEFAULT_SERVICE_PID_FILE).trimmed();
    config->deleteLater();

    auto domains = QDir(basePath + DEFAULT_SERVICE_DOMAINS_DIR).entryList(QDir::Files | QDir::NoDotAndDotDot);
    QString domainsAmount = "(" + QString::number(domains.size()) + ") ";
    if (domains.length() > 1) { /* more than just default localhost */
        domains.removeAll(DEFAULT_LOCAL_ADDRESS);
        QFile::remove(basePath + DEFAULT_SERVICE_DOMAINS_DIR + DEFAULT_SYSTEM_DOMAIN);
    }
    domain = domainsAmount + domains.first();

    port = readFileContents(basePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
    if (domain.isEmpty()) domain = "-";
    if (port.isEmpty()) port = "-";

    if(si){
        status = SERVICE_STATUS_INSTALLING;
    } else if(sv){
        status = SERVICE_STATUS_VALIDATING;
    } else if(sc){
        status = SERVICE_STATUS_CONFIGURING;
    } else if(isRunning) {
        if(pid.isEmpty()) {
            uint __port = registerFreeTcpPort(port.toUInt());
            if (port.toUInt() == __port) {
                // status = SERVICE_STATUS_WORKING;
                pid = "SCHD"; /* scheduler mode, means it's running and doing own jobs but isn't working in foreground all the time */
            } else {
                pid = " AUTO"; /* pid managment on service side */
            }
            status = SERVICE_STATUS_RUNNING;
        } else {
            status = SERVICE_STATUS_RUNNING;
        }
    } else if(se){
        status = SERVICE_STATUS_ERRORS;
    } else {
        status = SERVICE_STATUS_STOPPED;
        pid = "    -";
    }

    flags[0] = isRunning ? 'R' : '-';
    flags[1] = sv ? 'V' : '-';
    flags[2] = sc ? 'C' : '-';
    flags[3] = si ? 'I' : '-';
    flags[4] = se ? 'E' : '-';
    flags[5] = isAutostarted ? 'A' : '-';
    flags[6] = '\0';
}


void PanelService::start() {                   touch(basePath + START_TRIGGER_FILE);                   refresh();  }
void PanelService::startWithoutDeps() {        touch(basePath + START_WITHOUT_DEPS_TRIGGER_FILE);        refresh();  }
void PanelService::stop() {                    touch(basePath + STOP_TRIGGER_FILE);                    refresh();  }
void PanelService::stopWithoutDeps() {         touch(basePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);         refresh();  }
void PanelService::validate() {                touch(basePath + VALIDATE_TRIGGER_FILE);                refresh();  }
void PanelService::install() {                 touch(basePath + INSTALL_TRIGGER_FILE);                 refresh();  }
void PanelService::configure() {               touch(basePath + CONFIGURE_TRIGGER_FILE);               refresh();  }
void PanelService::reconfigure() {             touch(basePath + RECONFIGURE_TRIGGER_FILE);             refresh();  }
void PanelService::reconfigureWithoutDeps() {  touch(basePath + RECONFIGURE_WITHOUT_DEPS_TRIGGER_FILE);  refresh();  }
void PanelService::restart() {                 touch(basePath + RESTART_TRIGGER_FILE);                 refresh();  }
void PanelService::restartWithoutDeps() {      touch(basePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);      refresh();  }
void PanelService::reload() {                  touch(basePath + RELOAD_TRIGGER_FILE);                  refresh();  }
void PanelService::toggleAutostart() {
    QString file = basePath + AUTOSTART_TRIGGER_FILE;
    if (not QFile::exists(file)) touch(file);
    else QFile::remove(file);
    refresh();
}

bool PanelService::remove(){
    return removeDir(basePath, true);
}
