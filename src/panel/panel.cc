/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "panel.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));

    /* Logger setup - turn it off */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setDetailsLevel(Logger::Fatal);

    QString userHomeDir = getenv("HOME");
    signal(SIGINT, unixSignalHandler);
    // signal(SIGPIPE, SIG_IGN); /* ignore broken pipe signal */

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

    /* load services list */
    QDir home(userHomeDir + SOFTWARE_DATA_DIR);
    QFileInfoList apps = getApps(home);
    int APPS_NUMBER = apps.length();

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
        updateSSStatus(); /* will show status of service spawner */

        switch (ch) {
            case KEY_UP:
                if (APPS_NUMBER > 0) {
                    if (current_window_index > 0)
                        current_window_index -= 1;
                    else
                        current_window_index = 0;
                }
                break;

            case KEY_DOWN:
                if (APPS_NUMBER > 0) {
                    if (current_window_index < APPS_NUMBER - 1)
                        current_window_index += 1;
                    else
                        current_window_index = APPS_NUMBER - 1;
                }
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

            case KEY_F(7): /* Launch new service */ {
                    WINDOW *win = newwin(row/2, col/2, 5, 5);
                    wattron(win, COLOR_PAIR(6));
                    box(win, 1, 1);
                    wattroff(win, COLOR_PAIR(6));

                    QString newServiceName = "";
                    int ch = 0;
                    curs_set(1); /* cursor visible */

                    wattron(win, COLOR_PAIR(2));

                    auto services = availableServices();
                    mvwprintw(win, 1, 1, "Available service igniters: " + services.join(", ").toUtf8());
                    wattroff(win, COLOR_PAIR(2));

                    mvwprintw(win, 6, 1, "Enter service name to init: ");
                    while (ch != '\n' and ch != '\r') {
                        ch = wgetch(win);
                        newServiceName += ch;
                        mvwprintw(win, 6, 29, newServiceName.toUtf8());
                    }
                    mvwprintw(win, 6, 29, newServiceName.toUtf8());

                    if (services.contains(newServiceName.trimmed())) {
                        QDir home(userHomeDir + SOFTWARE_DATA_DIR);
                        getOrCreateDir(home.absolutePath() + "/" + newServiceName.trimmed()); /* NOTE: the only thing required is to make directory in ~/SoftwareData/newServiceName */
                        status = "Initialized service: " + newServiceName.trimmed();

                        /* reload services list */
                        apps = getApps(home);
                        APPS_NUMBER = apps.length();
                        current_window_index += 1;

                    } else {
                        status = "Not found service igniter called: " + newServiceName;
                    }

                    delwin(win);
                    curs_set(0); /* cursor invisible */
                    clear();
                    refresh();
                }
                break;

            case KEY_F(5): /* refresh */ {
                    apps = getApps(home);
                    APPS_NUMBER = apps.length();
                    clear();
                    refresh();
                    status = "Reloaded services list.";
                }
                break;

            case KEY_F(8): /* Remove current service */ {
                    if (QFile::exists(cursorAppDataDir + DEFAULT_SERVICE_RUNNING_FILE)) {
                        status = "You can't remove running service: " + cursorBaseDir.baseName();
                    } else {
                        status = "Are you sure you want to destroy data and configuration of service: " + cursorBaseDir.baseName() + "? (Y/y/↵ to confirm)";
                        printStatus(status);

                        auto key = getch();
                        switch (key) {
                            case 10:
                            case 'Y':
                            case 'y': {
                                    QFileInfo serviceDataDir(cursorAppDataDir);
                                    while (serviceDataDir.exists()) { // XXX: blocking and locking in case of "bad folder owner permissions".
                                        if (serviceDataDir.isDir() and
                                            serviceDataDir.isWritable() and
                                            serviceDataDir.isReadable() and
                                            serviceDataDir.isExecutable()) {

                                                // removeDir(cursorAppDataDir);
                                                removeDir(cursorAppDataDir);
                                                status = "Data dir removed: " + cursorAppDataDir;
                                                apps = getApps(home);
                                                APPS_NUMBER = apps.length();
                                                current_window_index -= 1;
                                                clear();
                                                break;

                                        } else {
                                            status = "Permissions failure. Check your user priviledges on your software data dir";
                                            break;
                                        }
                                        usleep(DEFAULT_PANEL_REFRESH_INTERVAL / 3);
                                    }
                                }
                                break;

                            default:
                                status = "Cancelled removing data dir: " + cursorAppDataDir;
                                break;
                        }
                    }
                }
                break;

            case 10: /* Show details */ {
                    QString outputFile = cursorAppDataDir + DEFAULT_SERVICE_LOG_FILE;
                    WINDOW *win = newwin(row - row/2 + 10, col - 10, 2, 5);
                    int modifier = 0; /* used to position log - scroll-like implementation */
                    bool loop = true;

                    while (loop) {
                        while (!kbhit()) {
                            QString contents = tail(outputFile, row/2 + 8, modifier);
                            wattron(win, COLOR_PAIR(6));
                            box(win, 1, 1);
                            mvwprintw(win, 0, 2, (cursorBaseDir.baseName() + " -> " + DEFAULT_SERVICE_LOG_FILE).toUtf8());
                            wattroff(win, COLOR_PAIR(6));
                            if (not contents.trimmed().isEmpty())
                                mvwprintw(win, 1, 1, contents.trimmed().toUtf8());
                            wrefresh(win);

                            usleep(DEFAULT_PANEL_REFRESH_INTERVAL);
                        }
                        int input = getch();
                        switch (input) {
                            case KEY_UP: {
                                modifier += DEFAULT_PANEL_SCROLL_SIZE;
                                }; break;

                            case KEY_DOWN: {
                                modifier -= DEFAULT_PANEL_SCROLL_SIZE;
                                }; break;

                            default:
                                loop = false;
                        }
                    }

                    delwin(win);
                    clear();
                } break;

            case 'A': /* Autotart */ {
                    QString autostartFile = cursorAppDataDir + DEFAULT_SERVICE_AUTOSTART_FILE;
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
                } else if(sr) {
                    if(pid.isEmpty()) {
                        uint __port = registerFreeTcpPort(port.toUInt());
                        if (port.toUInt() == __port) {
                            status = "Working...";
                            color = COLOR_PAIR(6);
                        } else {
                            status = "Running";
                            color = COLOR_PAIR(2);
                        }
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
                //flags[5] = ss ? 'S' : '-';


                mvprintw(y, x, " %s", flags);
                x += 6;


                if(sa)  mvprintw(y, x, "   YES       ");
                else    mvprintw(y, x, "             ");

                // clrtoeol();
            }

            standend();

            printStatus(status); /* print status - usually last command invoked */

            refresh();
            usleep(DEFAULT_PANEL_REFRESH_INTERVAL / 3);
            standend();
        }

        ch = getch();
        refresh();
    }
    endwin();
    return 0;
}
