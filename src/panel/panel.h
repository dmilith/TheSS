/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __SVDPANEL_H__
#define __SVDPANEL_H__


#include "../globals/globals.h"
#include "../service_spawner/logger.h"
#include "../service_spawner/utils.h"

#include <QtCore>
#include <curses.h>

#define SERVICE_STATUS_INSTALLING   0
#define SERVICE_STATUS_VALIDATING   1
#define SERVICE_STATUS_CONFIGURING  2
#define SERVICE_STATUS_WORKING      3
#define SERVICE_STATUS_RUNNING      4
#define SERVICE_STATUS_ERRORS       5
#define SERVICE_STATUS_STOPPED      6

#define C_DEFAULT COLOR_PAIR(1)
#define C_GREEN COLOR_PAIR(2)
#define C_STATUS COLOR_PAIR(5)
#define C_SS_STATUS_ON COLOR_PAIR(2)
#define C_SS_STATUS_OFF COLOR_PAIR(8)

#define C_STATUS              COLOR_PAIR(5)
#define C_STATUS_INSTALLING   COLOR_PAIR(6)
#define C_STATUS_VALIDATING   COLOR_PAIR(6)
#define C_STATUS_CONFIGURING  COLOR_PAIR(6)
#define C_STATUS_WORKING      COLOR_PAIR(6)
#define C_STATUS_RUNNING      COLOR_PAIR(2)
#define C_STATUS_ERRORS       COLOR_PAIR(8)
#define C_STATUS_STOPPED      (COLOR_PAIR(3) | A_BOLD)

#define C_CURRENT COLOR_PAIR(4)

extern bool TMUX;

class PanelService {
public:
    PanelService(QFileInfo baseDir);
    void start() const;
    void stop() const;
    void validate() const;
    void install() const;
    void configure() const;
    void restart() const;
    void toggleAutostart() const;
    bool remove() const;

    char flags[6];
    QString name, pid, domain, port;
    bool autostart;
    int status;
    QDir dir;
    bool isRunning;
    QFileInfo fileInfo;
    QString basePath;
};



template <class T>
class ScrollList {
public:
    ScrollList(QList<T> * items, int maxRows): items(items), maxRows(maxRows){}
    void display();
    void setItems(QList<T> * items);
    void reset(int maxRows);
    void key(int ch);
    const T * currentItem();
    virtual void displayHeader(){};
    virtual void displayItem(T item, int i, int num, bool current) = 0;
    virtual void displayEmptyItem(int i) = 0;
    virtual void displayFooter(){};

    QList<T> * items = NULL;
    int current = 0;
    int maxRows;
    int first = 0;
};


class AvailableServicesList : public ScrollList<QString> {
public:
    AvailableServicesList(QList<QString> * items, int maxRows, WINDOW * win);
    void displayHeader();
    void displayFooter();
    void setName(QString name);
    void displayItem(QString item, int i, int num, bool current);
    void displayEmptyItem(int i);
private:
    WINDOW *win;
    QString name;
    QList<QString> all;
};


class ServicesList : public ScrollList<PanelService> {
public:
    ServicesList(int maxRows): ScrollList(NULL, maxRows){}
    void displayHeader();
    void displayItem(PanelService service, int i, int num, bool current);
    void displayEmptyItem(int i);
};


class Panel {
public:
    Panel(QString user, QDir home, QDir ignitersDir);
    void refreshServicesList();
    QStringList * availableServices();
    bool isSSOnline();
    void setLogLevel(QString level);
    void shutdown();
    QString addService(QString name);

    QDir home;
    QString user;
    QDir ignitersDir;
    QList<PanelService> services;
    QStringList available;
};


class PanelGui {
public:
    PanelGui(Panel * panel):panel(panel){};
    void run();
    void displayHeader();
    void displayStatus();
    void displayFooter();
    void displayLog();
    void displayConfig();
    void newServiceDialog();
    void key(int ch);
    void removeCurrentService();
    bool confirm(QString msg);
    void cleanup();
    void pageUp();
    void pageDown();
    void helpDialog();
    void reload(int r, int c);
private:
    Panel * panel;
    void init();
    int rows, cols;
    QString status;
    int kbhit();
    ServicesList * servicesList;
    QString loggedServicePath = "";
    QString tailer = "most";
};

#endif
