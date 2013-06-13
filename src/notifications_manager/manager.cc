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

unsigned int rows = 0, cols = 0, x = 0, y = 1; /* x,y - cursor position */


void gui_init() {
    initscr();
    getmaxyx(stdscr, rows, cols); // get cols and rows amount
    cbreak();
    start_color();
    curs_set(0); // cursor invisible
    keypad(stdscr, TRUE);
    noecho();
    refresh();

    init_pair(1, COLOR_WHITE, COLOR_BLACK); // default
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // running service
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // stopped service
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // selected service
    init_pair(5, COLOR_CYAN, COLOR_BLACK); // status
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);
}


void gui_destroy() {
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
            QStringList notificationsByDate;
            notificationsByDate.reserve(notificationsByDateSource.size());
            reverse_copy(notificationsByDateSource.begin(), notificationsByDateSource.end(), back_inserter(notificationsByDate)); /* C++ list reverse.. huh */

            int indx = 0;
            Q_FOREACH(QString notify, notificationsByDate) {
                QString notificationFile = notificationsPrefix + "/" + notify;
                QString content = QString(readFileContents(notificationFile).c_str()).trimmed();
                if (notify.endsWith(".error")) {
                    attron(COLOR_PAIR(8));
                    mvprintw(y + indx + outdex, x, content.toUtf8());
                    attroff(COLOR_PAIR(8));
                }
                if (notify.endsWith(".warning")) {
                    attron(COLOR_PAIR(6));
                    mvprintw(y + indx + outdex, x, content.toUtf8());
                    attroff(COLOR_PAIR(6));
                }
                if (notify.endsWith(".notice")) {
                    attron(COLOR_PAIR(2));
                    mvprintw(y + indx + outdex, x, content.toUtf8());
                    attroff(COLOR_PAIR(2));
                }
                indx++;
            }
            outdex += indx;
        }
        refresh();
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
    fileAppender->setDetailsLevel(Logger::Debug);

    logInfo() << "Notifications manager v0.1.0";

    int input = '\0';
    while (input != 'q' and input != 'Q') {
        while (!kbhit()) {
            gatherNotifications();
            usleep(250000);
        }

        input = getch();
        switch (input) {
            case KEY_F(5): {

                refresh();
                return 0;
            }
        }
    }

    gui_destroy();
    return 0;
}
