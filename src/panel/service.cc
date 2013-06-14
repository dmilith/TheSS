/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

 #include "panel.h"

PanelService::PanelService(QFileInfo baseDir){
    this->fileInfo = baseDir;
    name = baseDir.baseName();
    basePath = baseDir.absolutePath() + "/" + baseDir.baseName();
    this->dir = QDir(basePath);
    isRunning = QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE);
    bool sv = QFile::exists(basePath + DEFAULT_SERVICE_VALIDATING_FILE);
    bool sc = QFile::exists(basePath + DEFAULT_SERVICE_CONFIGURING_FILE);
    bool si = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE);
    bool se = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE); // XXX: FIXME: no more .errors file!!
    autostart = QFile::exists(basePath + DEFAULT_SERVICE_AUTOSTART_FILE);

    pid = QString(readFileContents(basePath + DEFAULT_SERVICE_PID_FILE).c_str()).trimmed();
    domain = QString(readFileContents(basePath + DEFAULT_SERVICE_DOMAIN_FILE).c_str()).trimmed();
    port = QString(readFileContents(basePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).c_str()).trimmed();
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
                status = SERVICE_STATUS_WORKING;
            } else {
                status = SERVICE_STATUS_RUNNING;
            }
            pid = "    -";
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

void PanelService::start() const { touch(dir.absoluteFilePath(".start")); }
void PanelService::stop() const { touch(dir.absoluteFilePath(".stop")); }
void PanelService::validate() const { touch(dir.absoluteFilePath(".validate")); }
void PanelService::install() const { touch(dir.absoluteFilePath(".install")); }
void PanelService::configure() const { touch(dir.absoluteFilePath(".configure")); }
void PanelService::reconfigure() const { touch(dir.absoluteFilePath(".reconfigure")); }
void PanelService::restart() const { touch(dir.absoluteFilePath(".restart")); }
void PanelService::toggleAutostart() const {
    QString file = dir.absoluteFilePath(".autostart");
    if (not QFile::exists(file)) touch(file);
    else QFile::remove(file);
}

bool PanelService::remove() const {
    while (fileInfo.exists()) { // XXX: blocking and locking in case of "bad folder owner permissions".
        if (fileInfo.isWritable() &&
            fileInfo.isReadable() &&
            fileInfo.isExecutable()) {
            removeDir(basePath);
            return true;
        } else {
            return false;
        }
    }

    return true;
}
