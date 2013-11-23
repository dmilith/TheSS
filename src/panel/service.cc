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

    this->log = new Tail(this, basePath, "service.log");
    this->conf  = new Tail(this, basePath, "service.conf");
    this->env   = new Tail(this, basePath, "service.env");

    refresh();
}

void PanelService::refresh(){

    // logDebug() << "new tail";
    // logTail(basePath, "service.log");

    isRunning = QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE);
    bool sv = QFile::exists(basePath + DEFAULT_SERVICE_VALIDATING_FILE);
    bool sc = QFile::exists(basePath + DEFAULT_SERVICE_CONFIGURING_FILE);
    bool si = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE);
    bool se = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE); // XXX: FIXME: no more .errors file!!
    autostart = QFile::exists(basePath + DEFAULT_SERVICE_AUTOSTART_FILE);

    pid = readFileContents(basePath + DEFAULT_SERVICE_PID_FILE).trimmed();
    domain = readFileContents(basePath + DEFAULT_SERVICE_DOMAIN_FILE).trimmed();
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
                pid = "SCHED"; /* scheduler mode, means it's running and doing own jobs but isn't working in foreground all the time */
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
    flags[5] = '\0';
}


void PanelService::start() {                   touch(dir.absoluteFilePath(".start"));                   refresh();  }
void PanelService::startWithoutDeps() {        touch(dir.absoluteFilePath(".startWithoutDeps"));        refresh();  }
void PanelService::stop() {                    touch(dir.absoluteFilePath(".stop"));                    refresh();  }
void PanelService::stopWithoutDeps() {         touch(dir.absoluteFilePath(".stopWithoutDeps"));         refresh();  }
void PanelService::validate() {                touch(dir.absoluteFilePath(".validate"));                refresh();  }
void PanelService::install() {                 touch(dir.absoluteFilePath(".install"));                 refresh();  }
void PanelService::configure() {               touch(dir.absoluteFilePath(".configure"));               refresh();  }
void PanelService::reconfigure() {             touch(dir.absoluteFilePath(".reconfigure"));             refresh();  }
void PanelService::reconfigureWithoutDeps() {  touch(dir.absoluteFilePath(".reconfigureWithoutDeps"));  refresh();  }
void PanelService::restart() {                 touch(dir.absoluteFilePath(".restart"));                 refresh();  }
void PanelService::restartWithoutDeps() {      touch(dir.absoluteFilePath(".restartWithoutDeps"));      refresh();  }
void PanelService::reload() {                  touch(dir.absoluteFilePath(".reload"));                  refresh();  }
void PanelService::toggleAutostart() {
    QString file = dir.absoluteFilePath(".autostart");
    if (not QFile::exists(file)) touch(file);
    else QFile::remove(file);
    refresh();
}

bool PanelService::remove(){
    auto prc = new SvdProcess("SS", getuid(), false);
    prc->spawnProcess("rm -rf " + basePath);
    prc->waitForFinished(10);

    return true;
}
