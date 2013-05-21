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
    QList<WINDOW*> windows;

    int ch = 'p';
    int current_window_index = 0;

    QDir home(userHomeDir + SOFTWARE_DATA_DIR);
    QFileInfoList apps = home.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoDot);
    int APPS_NUMBER = apps.length();

    for (int i = 0; i < APPS_NUMBER; i++) {
        windows << newwin(6, col / APPS_NUMBER * 4, 6 * i + 1, 1);
    }

    /* selected color */
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

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

        for (int i = 0; i < APPS_NUMBER; i++) {
            box(windows.at(i), 0 , 0);
            if (current_window_index == i) {
                wattron(windows.at(i), COLOR_PAIR(3));
            } else {
                wattron(windows.at(i), COLOR_PAIR(2));
            }
            mvwprintw(windows.at(i), 1, 1, "Service name:");
            mvwprintw(windows.at(i), 2, 1, "Service port:");
            mvwprintw(windows.at(i), 3, 1, "Service domain:");
            mvwprintw(windows.at(i), 4, 1, "Service status:");

            wattron(windows.at(i), COLOR_PAIR(1));
            QFileInfo baseDir = apps.at(i);
            QString basePath = baseDir.absolutePath() + "/" + baseDir.baseName();
            mvwprintw(windows.at(i), 1, 20, baseDir.baseName().toUtf8());
            mvwprintw(windows.at(i), 2, 20, QString(readFileContents(basePath + DEFAULT_SERVICE_PORTS_FILE).c_str()).trimmed().toUtf8());
            mvwprintw(windows.at(i), 3, 20, QString(readFileContents(basePath + DEFAULT_SERVICE_DOMAIN_FILE).c_str()).trimmed().toUtf8());
            if (QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE))
                mvwprintw(windows.at(i), 4, 20, "Running    ");
            else
                mvwprintw(windows.at(i), 4, 20, "Not running");

            // mvprintw(1, 1, QString::number(APPS_NUMBER).toUtf8());
            wrefresh(windows.at(i));
        }

        ch = getch();
        refresh();
    }
    endwin();
    return 0;
}