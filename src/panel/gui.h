#ifndef __PANEL_GUI_H__
#define __PANEL_GUI_H__

#include <QtCore>
#include <ncurses.h>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "panel.h"

class PanelGui : public QObject {
    Q_OBJECT

public:
    PanelGui(Panel * panel):panel(panel){};
    void key(int ch);
    void helpDialog();
    void displayStatus();
    void displayFooter();
    void reload(int r, int c);
    void newServiceDialog();
    void removeCurrentService();
    bool confirm(QString msg);
    void init();
    QString newEntry(QString defaultEntry);

public slots:
    void run();
    void readInput();
    void display();
    void displayLog();
    void displayConfig();
    void displayEnv();
    void displaySSLog();
    void tailUpdate();
    void gatherNotifications();

signals:
    void quit();
    void getInput();


//     PanelGui(Panel * panel):panel(panel){};
//     void run();
//     QString newDomain();
//     void displayLog();
//     void displayConfig();
//     void displayEnv();
//     void displayFile(QString file);
//     void searchLog();
// private:

private:
    void cleanup();
    int kbhit();

    void displayHeader();
    void tailScroll(int n);
    void tailToggleWrap();
    void tailReset();

    int rows, cols;

    Panel * panel;
    ServicesList * servicesList;
    QString status;
    WINDOW * mainWindow;
    WINDOW * logWindow;
    WINDOW * notificationWindow;
    Tail * tail = NULL;
    const PanelService * recentService = NULL;

    int notificationRows = NOTIFICATIONS_LAST_SHOWN + NOTIFICATIONS_OPTIONS_BAR_HEIGHT;
};


#endif

// class PanelGui {
// public:

//     Panel * panel;
//     void init();
//     int kbhit();
//     QString loggedServicePath = "";
//     bool wrapLines = true;
// };
