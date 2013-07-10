/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __PANEL_H__
#define __PANEL_H__


#include <QtCore>
#include <curses.h>
// #include "../globals/globals.h"
// #include "../service_spawner/logger.h"
#include "../service_spawner/utils.h"
#include "../service_spawner/file_events_manager.h"


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
#define C_SCROLLING COLOR_PAIR(7)


// Forward referecnes, C++ you're such a moron
class Panel;
class ServicesList;
class PanelGui;
class PanelService;
class Tail;

#include "service.h"
#include "tail.h"
#include "gui.h"

class Panel : public QObject {
    Q_OBJECT

public:
    Panel(QString user, QDir home, QDir ignitersDir);
    void refreshServicesList();
    void setLogLevel(QString level);
    void start();

signals:
    void refreshed();

public slots:
    void refresh();
    void onDirectoryChanged(QString dir);
    bool isSSOnline();
    QStringList * availableServices();
    void shutdown();
    QString addService(QString name);
public:
    void setGui(PanelGui * gui);

    QDir home;
    QString user;
    QDir ignitersDir;
    QList<PanelService *> services;
    SvdFileEventsManager * eventsManager;
    PanelGui * gui;
    QStringList available;
    QMutex refreshMutex;
};



template <class T>
class ScrollList {
public:
    ScrollList(QList<T> * items, int maxRows, WINDOW * win): items(items), maxRows(maxRows), win(win){}
    void display();
    void setItems(QList<T> * items);
    void reset(int maxRows);
    void key(int ch);
    T currentItem();
    virtual void displayHeader(){};
    virtual void displayItem(T item, int i, int num, bool current) = 0;
    virtual void displayEmptyItem(int i) = 0;
    virtual void displayFooter(){};

    QList<T> * items = NULL;
    int current = 0;
    int maxRows;
    int first = 0;
    WINDOW * win;
};


class AvailableServicesList : public ScrollList<QString> {
public:
    AvailableServicesList(QList<QString> * items, int maxRows, WINDOW * win, QString aHeader = "Select service:");
    void displayHeader();
    void displayFooter();
    void setName(QString name);
    void displayItem(QString item, int i, int num, bool current);
    void displayEmptyItem(int i);
private:
    QString name, header;
    QList<QString> all;
};


class ServicesList : public ScrollList<PanelService *> {
public:
    ServicesList(int maxRows, WINDOW * win): ScrollList(NULL, maxRows, win){}
    void displayHeader();
    void displayItem(PanelService * service, int i, int num, bool current);
    void displayEmptyItem(int i);
    void setCurrent(QString selected);
};

#endif
