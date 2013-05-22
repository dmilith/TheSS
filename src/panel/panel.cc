/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "../globals/globals.h"
#include "../service_spawner/logger.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/service_watcher.h"
#include "../service_spawner/user_watcher.h"
#include "../service_spawner/utils.h"

#include <QtCore>

#include <curses.h>
// #include <panel.h>
// #include <menu.h>
// #include <form.h>


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


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    // QStringList args = app.arguments();
    // QRegExp rxEnableDebug("-d");
    // QRegExp rxEnableTrace("-t");
    // QRegExp rxPrintVersion("-v");

    /* Logger setup - turn it off */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setDetailsLevel(Logger::Fatal);

    QString userHomeDir = getenv("HOME");
    signal(SIGINT, unixSignalHandler);
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

    int row, col; /* rows and cols of terminal */

    initscr();
    getmaxyx(stdscr, row, col); /* get cols and rows amount */
    cbreak();
    start_color();
    curs_set(0); /* cursor invisible */
    keypad(stdscr, TRUE);
    noecho();
    refresh();

    int ch = 'p';
    int current_window_index = 0;

    QDir home(userHomeDir + SOFTWARE_DATA_DIR);
    QFileInfoList apps = home.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoDot);
    int APPS_NUMBER = apps.length();

    /* selected color */
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    attron(COLOR_PAIR(1));
    mvprintw(1, 1, "| name       | softwareName | pid  | port  | domain     | r?  | v?  | s?  | t?  | c?  | i? |");
    attroff(COLOR_PAIR(1));

    while (ch != 'q') {
        switch (ch) {
            case KEY_UP:
                if (current_window_index > 0)
                    current_window_index -= 1;
                else
                    current_window_index = 0;
                break;

            case KEY_DOWN:
                if (current_window_index < APPS_NUMBER - 1)
                    current_window_index += 1;
                else
                    current_window_index = APPS_NUMBER - 1;
                break;
        }

        while (!kbhit()) {
            for (int i = 0; i < APPS_NUMBER; i++) {
                QFileInfo baseDir = apps.at(i);
                QString basePath = baseDir.absolutePath() + "/" + baseDir.baseName();

                if (current_window_index == i) {
                    attron(COLOR_PAIR(3));
                } else {
                    attron(COLOR_PAIR(2));
                }

                mvprintw(i + 2, 1, baseDir.baseName().toUtf8());
                mvprintw(i + 2, 30, QString(readFileContents(basePath + DEFAULT_SERVICE_PORTS_FILE).c_str()).trimmed().toUtf8());
                mvprintw(i + 2, 45, QString(readFileContents(basePath + DEFAULT_SERVICE_DOMAIN_FILE).c_str()).trimmed().toUtf8());
                if (QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE))
                    mvprintw(i + 2, 60, "Running    ");
                else
                    mvprintw(i + 2, 60, "Not running");

            }

            mvprintw(1, 70, QString::number(APPS_NUMBER).toUtf8());
            refresh();
            usleep(100000);
        }

        ch = getch();
        refresh();
    }
    endwin();
    return 0;
}