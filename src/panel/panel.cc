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

    if (APPS_NUMBER == 0) {
        endwin();
        logError() << "No services to watch!";
        exit(1);
    }

    /* selected color */
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // default
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // running service
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // stopped service
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // selected service
    init_pair(5, COLOR_CYAN, COLOR_BLACK); // status
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);

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

            case 10: /* Show details */ {
                    QString outputFile = cursorAppDataDir + "/service.log";
                    QString outputFile2 = cursorAppDataDir + "/.output.log";
                    WINDOW *win = newwin(row - row/2 + 10, col - 10, 2, 5);

                    while (!kbhit()) {
                        QString contents = tail(outputFile, row/2 + 8);
                        wattron(win, COLOR_PAIR(6));
                        box(win, 1, 1);
                        mvwprintw(win, 0, 2, (cursorBaseDir.baseName() + " -> service.log").toUtf8());
                        wattroff(win, COLOR_PAIR(6));
                        if (not contents.trimmed().isEmpty())
                            mvwprintw(win, 1, 1, contents.trimmed().toUtf8());
                        wrefresh(win);

                        usleep(300000);
                    }
                    getch();
                    delwin(win);
                    clear();
                } break;

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
            /* write app header */
            attron(COLOR_PAIR(1));
            QString info = "Conrol Panel, version: " + QString(APP_VERSION) + ". " + QString(COPYRIGHT);
            mvprintw(0, 1, info.toUtf8());
            mvprintw(0, 90, "Services: " + QString::number(APPS_NUMBER).toUtf8());
            mvprintw(3, 0, " Name                        PID Address                Status        Flags    Autostart?");
            attroff(COLOR_PAIR(1));

            char flags[6];
            flags[5] = '\0';

            for (int i = 0; i < APPS_NUMBER; i++) {
                QFileInfo baseDir = apps.at(i);
                QString basePath = baseDir.absolutePath() + "/" + baseDir.baseName();
                bool sr = QFile::exists(basePath + DEFAULT_SERVICE_RUNNING_FILE);
                bool sv = QFile::exists(basePath + DEFAULT_SERVICE_VALIDATING_FILE);
                bool sc = QFile::exists(basePath + DEFAULT_SERVICE_CONFIGURING_FILE);
                bool si = QFile::exists(basePath + DEFAULT_SERVICE_INSTALLING_FILE);
                bool se = QFile::exists(basePath + DEFAULT_SERVICE_ERRORS_FILE);
                bool ss = QFile::exists(basePath + "/.stop");
                bool sa = QFile::exists(basePath + DEFAULT_SERVICE_AUTOSTART_FILE);
                int x = 0, y = i + 4;

                QString pid = QString(readFileContents(basePath + DEFAULT_SERVICE_PID_FILE).c_str()).trimmed();
                QString domain = QString(readFileContents(basePath + DEFAULT_SERVICE_DOMAIN_FILE).c_str()).trimmed();
                QString port = QString(readFileContents(basePath + DEFAULT_SERVICE_PORTS_FILE).c_str()).trimmed();


                QString status;
                int color;

                if(si){
                    status = "Installing...";
                    color = COLOR_PAIR(6);
                } else if(sv){
                    status = "Validating...";
                    color = COLOR_PAIR(6);
                } else if(sc){
                    status = "Configuring...";
                    color = COLOR_PAIR(6);
                } else if(ss){
                    status = "Stopping...";
                    color = COLOR_PAIR(7);
                } else if(sr) {
                    if(pid.isEmpty()){
                        status = "Starting...";
                        color = COLOR_PAIR(6);
                        pid = "    -";
                    } else {
                        status = "Running";
                        color = COLOR_PAIR(2);
                    }
                } else if(se){
                    status = "Errors";
                    color = COLOR_PAIR(8);
                } else {
                    status = "Stopped";
                    color = COLOR_PAIR(3) | A_BOLD;
                    pid = "    -";
                }

                standend();

                if(current_window_index == i){
                    attron(COLOR_PAIR(4));
                } else {
                    attron(color);
                }

                /* name */
                mvprintw(y, x, " %-26s", baseDir.baseName().toUtf8().data());
                x += 27;

                /* pid */
                mvprintw(y, x, "%5s", pid.toUtf8().data());
                x += 5;

                /* domain:port */
                mvprintw(y, x, " %-22s", (domain + ":" + port).toUtf8().data());
                x += 23;


                /* flags & status */
                mvprintw(y, x, " %-14s", status.toUtf8().data());
                x += 14;

                flags[0] = sr ? 'R' : '-';
                flags[1] = sv ? 'V' : '-';
                flags[2] = sc ? 'C' : '-';
                flags[3] = si ? 'I' : '-';
                flags[4] = se ? 'E' : '-';
                flags[5] = ss ? 'S' : '-';


                mvprintw(y, x, " %s", flags);
                x += 7;


                if(sa)  mvprintw(y, x, "   YES       ");
                else    mvprintw(y, x, "             ");

                // clrtoeol();
            }

            standend();
            /* print status - usually last command invoked */
            attron(COLOR_PAIR(5));
            QString statusContent = "Status: " + status;
            mvprintw(1, 1, statusContent.toUtf8());
            attroff(COLOR_PAIR(5));

            refresh();
            usleep(100000);
            standend();
        }

        ch = getch();
        refresh();
    }
    endwin();
    return 0;
}
