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
    QString status = "Watching";
    int current_window_index = 0;

    QDir home(userHomeDir + SOFTWARE_DATA_DIR);
    QFileInfoList apps = home.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoDot);
    int APPS_NUMBER = apps.length();

    /* selected color */
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);

    attron(COLOR_PAIR(1));
    QString info = "Conrol Panel, version: " + QString(APP_VERSION) + ". " + QString(COPYRIGHT);
    mvprintw(0, 1, info.toUtf8());
    mvprintw(0, 90, "Services: " + QString::number(APPS_NUMBER).toUtf8());
    mvprintw(3, 1, "Name                      PID    Address                Running?  Validating?  Configuring?  Installing?  Errors?  Autostart?");
    attroff(COLOR_PAIR(1));

    while (ch != 'q') {
        QFileInfo cursorBaseDir = apps.at(current_window_index);
        QString cursorAppDataDir = cursorBaseDir.absolutePath() + "/" + cursorBaseDir.baseName();

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

            case KEY_F(1): /* Trace */
                touch(QString(getenv("HOME")) + "/.trace");
                status = "Triggered log level change to: Trace";
                break;

            case KEY_F(2): /* Debug */
                touch(QString(getenv("HOME")) + "/.debug");
                status = "Triggered log level change to: Debug";
                break;

            case KEY_F(3): /* Info */
                touch(QString(getenv("HOME")) + "/.info");
                status = "Triggered log level change to: Info";
                break;

            case KEY_F(4): /* Warn */
                touch(QString(getenv("HOME")) + "/.warn");
                status = "Triggered log level change to: Warning";
                break;

            case 'A': /* Autotart */ {
                    QString autostartFile = cursorAppDataDir + "/.autostart";
                    if (not QFile::exists(autostartFile))
                        touch(autostartFile);
                    else
                        QFile::remove(autostartFile);
                }
                status = "Triggered autostart of application: " + cursorBaseDir.baseName();
                break;

            case 'S': /* Start */
                touch(cursorAppDataDir + "/.start");
                status = "Triggered start of application: " + cursorBaseDir.baseName();
                break;

            case 'T': /* sTop */
                touch(cursorAppDataDir + "/.stop");
                status = "Triggered stop of application: " + cursorBaseDir.baseName();
                break;

            case 'V': /* Validate */
                touch(cursorAppDataDir + "/.validate");
                status = "Triggered validation of application: " + cursorBaseDir.baseName();
                break;

            case 'I': /* Install */
                touch(cursorAppDataDir + "/.install");
                status = "Triggered installation of application: " + cursorBaseDir.baseName();
                break;

            case 'C': /* Configure */
                touch(cursorAppDataDir + "/.configure");
                status = "Triggered configuration of application: " + cursorBaseDir.baseName();
                break;

            case 'R': /* Restart */
                touch(cursorAppDataDir + "/.restart");
                status = "Triggered restart of application: " + cursorBaseDir.baseName();
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

                /* name */
                mvprintw(i + 4, 1, baseDir.baseName().toUtf8());

                /* pid */
                QString pid = QString(readFileContents(basePath + DEFAULT_SERVICE_PID_FILE).c_str()).trimmed();
                if (pid.isEmpty())
                    mvprintw(i + 4, 27, "-     ");
                else
                    mvprintw(i + 4, 27, pid.toUtf8());

                /* domain:port */
                QString domain = QString(readFileContents(basePath + DEFAULT_SERVICE_DOMAIN_FILE).c_str()).trimmed();
                QString port = QString(readFileContents(basePath + DEFAULT_SERVICE_PORTS_FILE).c_str()).trimmed();
                mvprintw(i + 4, 34, (domain + ":" + port).toUtf8());

                /* running? */
                if (QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE))
                    mvprintw(i + 4, 57, "YES");
                else
                    mvprintw(i + 4, 57, "NO ");

                /* validating? */
                if (QFile::exists(basePath + DEFAULT_SERVICE_VALIDATING_FILE))
                    mvprintw(i + 4, 67, "YES");
                else
                    mvprintw(i + 4, 67, "NO ");

                /* configuring? */
                if (QFile::exists(basePath + DEFAULT_SERVICE_CONFIGURING_FILE))
                    mvprintw(i + 4, 80, "YES");
                else
                    mvprintw(i + 4, 80, "NO ");

                /* installing? */
                if (QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE))
                    mvprintw(i + 4, 94, "YES");
                else
                    mvprintw(i + 4, 94, "NO ");

                /* errors? */
                if (QFile::exists(basePath + DEFAULT_SERVICE_ERRORS_FILE))
                    mvprintw(i + 4, 107, "YES");
                else
                    mvprintw(i + 4, 107, "NO ");

                /* autostart? */
                if (QFile::exists(basePath + DEFAULT_SERVICE_AUTOSTART_FILE))
                    mvprintw(i + 4, 116, "YES");
                else
                    mvprintw(i + 4, 116, "NO ");
            }

            /* print status - usually last command invoked */
            attron(COLOR_PAIR(5));
            QString statusContent = "Status: " + status;

            /* a trick or hack, to clean screen after longer status messages in previous status content */
            for (int ind = 1; ind < col; ind++) /* col is number of columns available on terminal */
                statusContent += ' ';

            mvprintw(1, 1, statusContent.toUtf8());
            attroff(COLOR_PAIR(5));

            refresh();
            usleep(100000);
        }

        ch = getch();
        refresh();
    }
    endwin();
    return 0;
}