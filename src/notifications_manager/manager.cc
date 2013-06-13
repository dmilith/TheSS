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


unsigned int rows = 0, cols = 0, x = 0, y = 0; /* x,y - cursor position */
WINDOW *win = NULL;


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

    /* iterate through user software to find software notifications */
    int outdex = 0;
    Q_FOREACH(QString service, userSoftwareList) {

        logTrace() << "Found service:" << service;
        QString notificationsPrefix = userSoftwarePrefix + "/" + service + NOTIFICATIONS_DATA_DIR;
        if (QDir().exists(notificationsPrefix)) {
            QStringList notificationsByDateSource = QDir(notificationsPrefix).entryList(QDir::Files, QDir::Time);
            QStringList notificationsByDate = notificationsByDateSource;

            int indx = 0;
            Q_FOREACH(QString notify, notificationsByDate) {
                QString notificationFile = notificationsPrefix + "/" + notify;
                QString content = QString(readFileContents(notificationFile).c_str()).trimmed();
                if (notify.endsWith(".error")) {
                    wattron(win, COLOR_PAIR(8));
                    mvwprintw(win, indx + outdex, x, content.toUtf8());
                    wattroff(win, COLOR_PAIR(8));
                }
                if (notify.endsWith(".warning")) {
                    wattron(win, COLOR_PAIR(6));
                    mvwprintw(win, indx + outdex, x, content.toUtf8());
                    wattroff(win, COLOR_PAIR(6));
                }
                if (notify.endsWith(".notice")) {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, indx + outdex, x, content.toUtf8());
                    wattroff(win, COLOR_PAIR(2));
                }
                indx++;
            }
            outdex += indx;
        }
        wrefresh(win);
        // refresh();
    }
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
