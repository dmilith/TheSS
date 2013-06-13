/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include <QtCore>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../service_spawner/utils.h"
#include <ncurses.h>
#include <stdlib.h>


int rows = 0, cols = 0, x = 0, y = 0; /* x,y - cursor position */
WINDOW *win = NULL;


#define NOTIFICATION_LEVEL_ERROR    0
#define NOTIFICATION_LEVEL_WARNING  1
#define NOTIFICATION_LEVEL_NOTICE   2

struct Notification {
    int level;
    QString content;
    QDateTime time;
};

bool NotificationLessThan(const Notification &a, const Notification &b){
    return a.time < b.time;
}


void gui_init() {
    initscr();
    start_color();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0); // cursor invisible
    noecho();

    getmaxyx(stdscr, rows, cols); // get cols and rows amount
    // y = rows - 1; /* set cursor to bottom */
    // win = newwin(rows, cols, 1, 1);

    init_pair(1, COLOR_WHITE, COLOR_BLACK); // default
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // running service
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // stopped service
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // selected service
    init_pair(5, COLOR_CYAN, COLOR_BLACK); // status
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);

    win = newwin(rows, cols, 0, 0);
    wattron(win, COLOR_PAIR(6));
    box(win, 1, 1);
    wattroff(win, COLOR_PAIR(6));
    scrollok(win, FALSE);

    refresh();
}


void gui_destroy() {
    delwin(win);
    endwin();
}


int kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}


void gatherNotifications() {
    QString userSoftwarePrefix = QString(getenv("HOME")) + QString(SOFTWARE_DATA_DIR);
    auto userSoftwareList = QDir(userSoftwarePrefix).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QList<Notification> notifications;

    /* iterate through user software to find software notifications */
    Q_FOREACH(QString service, userSoftwareList) {
        QString notificationsPrefix = userSoftwarePrefix + "/" + service + NOTIFICATIONS_DATA_DIR;
        if (QDir().exists(notificationsPrefix)) {
            auto files = QDir(notificationsPrefix).entryInfoList(QDir::Files, QDir::Time);

            Q_FOREACH(auto file, files) {
                Notification n;
                QString ext = file.suffix();

                if (ext == "error")         n.level = NOTIFICATION_LEVEL_ERROR;
                else if (ext == "warning")  n.level = NOTIFICATION_LEVEL_WARNING;
                else if(ext == "notice")    n.level = NOTIFICATION_LEVEL_NOTICE;

                n.content = QString(readFileContents(file.absoluteFilePath()).c_str()).trimmed();
                n.time = file.created();
                notifications.append(n);
            }
        }
        // refresh();
    }

    // Sort
    qSort(notifications.begin(), notifications.end(), NotificationLessThan);

    // Display
    int s = notifications.size();
    int start = max(0, s - rows);
    int stop = min(s, rows);

    logTrace() << "s: " << s << " start: " << start << " stop: " << stop;

    int i=0;
    for(; i<stop; i++){
        Notification n = notifications.at(i+start);
        switch(n.level){
            case NOTIFICATION_LEVEL_ERROR:
                wattron(win, COLOR_PAIR(8));
                break;
            case NOTIFICATION_LEVEL_WARNING:
                wattron(win, COLOR_PAIR(6));
                break;
            case NOTIFICATION_LEVEL_NOTICE:
                wattron(win, COLOR_PAIR(2));
                break;
        }

        mvwprintw(win, i, x, n.content.toUtf8());
        wclrtoeol(win);
    }
    for(; i<rows;i++){
        wmove(win, i, x);
        wclrtoeol(win);
    }

    wrefresh(win);
}


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    QStringList args = app.arguments();
    gui_init();

    /* Logger setup */
    QString logFile = QString(getenv("HOME")) + "/.notifications-manager.log";
    QFile::remove(logFile);

    FileAppender *fileAppender = new FileAppender(logFile);
    // fileAppender->ansiColors = false;
    fileAppender->setFormat("%m\n");
    Logger::registerAppender(fileAppender);
    fileAppender->setDetailsLevel(Logger::Trace);

    logInfo() << "Notifications manager v0.2.0";

    int input = '\0';
    while (input != 'q' and input != 'Q') {
        while (!kbhit()) {
            gatherNotifications();
            // wrefresh(win);
            usleep(250000);
        }

        input = getch();
        switch (input) {
            case KEY_F(5): {

                refresh();
            }
        }
        // wrefresh(win);
    }

    gui_destroy();
    return 0;
}
