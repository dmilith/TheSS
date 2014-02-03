/**
 *  @author teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __PANEL_SERVICE_H__
#define __PANEL_SERVICE_H__

#include "panel.h"

class PanelService {

public:
    PanelService(Panel * panel, QFileInfo baseDir);
    void panelAssert(bool predicate, const char* where, const char* message);
    void start();
    void startWithoutDeps();
    void stop();
    void stopWithoutDeps();
    void validate();
    void install();
    void configure();
    void reconfigure();
    void reconfigureWithoutDeps();
    void restart();
    void restartWithoutDeps();
    void reload();
    void toggleAutostart();
    bool remove();
    void refresh();

    char flags[7];
    QString name, pid, domain, port;
    bool autostart;
    int status;
    QDir dir;
    bool isRunning;
    QFileInfo baseDir;
    QString basePath;
    Panel * panel;
    Tail * log;
    Tail * conf;
    Tail * env;
};

#endif
