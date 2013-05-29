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
    int app_index_first = 0;
    QDir home(userHomeDir + SOFTWARE_DATA_DIR);

    /* sanity check */
    getOrCreateDir(getenv("HOME") + QString(SOFTWARE_DATA_DIR));
    getOrCreateDir(QString(getenv("HOME")) + "/Igniters/Services");

    /* selected color */
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // default
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // running service
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // stopped service
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // selected service
    init_pair(5, COLOR_CYAN, COLOR_BLACK); // status
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);

    /* reload services list */
    QFileInfoList apps = getApps(home);
    int APPS_NUMBER = apps.length();
    int max_rows = min(APPS_NUMBER, row-4);
    int modifier = 0; /* log viewer position modifier */

    while (ch != 'q') {
        QFileInfo cursorBaseDir;
        QString cursorAppDataDir;

        if (APPS_NUMBER == 0) {
            status = "No initialized services found in data directory. Hit F7 to add new.";
        } else {
            cursorBaseDir = apps.at(current_window_index);
            cursorAppDataDir = cursorBaseDir.absolutePath() + "/" + cursorBaseDir.baseName();
        }

        switch (ch) {
            case KEY_UP:
                if (APPS_NUMBER > 0) {
                    if (current_window_index > 0)
                        current_window_index -= 1;
                    else
                        current_window_index = 0;

                    if(current_window_index < app_index_first)
                        app_index_first = current_window_index;
                }
                modifier = 0; /* reset modifier after changed log source */
                break;

            case KEY_DOWN:
                if (APPS_NUMBER > 0) {
                    if (current_window_index < APPS_NUMBER - 1)
                        current_window_index += 1;
                    else
                        current_window_index = APPS_NUMBER - 1;

                    if(current_window_index >= app_index_first + max_rows)
                        app_index_first++;

                }
                modifier = 0; /* reset modifier after changed log source */
                break;

            case KEY_PPAGE: { /* page up */
                modifier += DEFAULT_PANEL_SCROLL_SIZE;
                }; break;

            case KEY_NPAGE: { /* page down */
                modifier -= DEFAULT_PANEL_SCROLL_SIZE;
                }; break;


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

                    if (not QDir(getenv("HOME") + QString(SOFTWARE_DATA_DIR)).exists() /* service isn't already defined */) {
                        if (services.contains(newServiceName.trimmed()) /* is available */) {
                            QDir home(userHomeDir + SOFTWARE_DATA_DIR);
                            getOrCreateDir(home.absolutePath() + "/" + newServiceName.trimmed()); /* NOTE: the only thing required is to make directory in ~/SoftwareData/newServiceName */
                            status = "Initialized service: " + newServiceName.trimmed();

                            /* reload services list */
                            apps = getApps(home);
                            APPS_NUMBER = apps.length();
                            if (APPS_NUMBER == 1) /* NOTE: when adding first app, index remains the same => 0 */
                                current_window_index = 0;
                            else
                                current_window_index += 1;

                        } else
                            status = "Not found service igniter called: " + newServiceName;

                    } else
                        status = "Already defined service called: " + newServiceName;

                    delwin(win);
                    curs_set(0); /* cursor invisible */
                    clear();
                    refresh();
                }
                break;

            case KEY_F(5): /* refresh */ {
                    apps = getApps(home);
                    APPS_NUMBER = apps.length();
                    getmaxyx(stdscr, row, col); /* get cols and rows amount - f.e. after terminal resize */
                    app_index_first = 0;
                    current_window_index = 0;
                    clear();
                    refresh();
                    status = "Reloaded services list.";
                }
                break;

            case KEY_F(8): /* Remove current service */ {
                    if (APPS_NUMBER == 0) {
                        status = "You can't remove non existant service.";
                        break;
                    }
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
                                                if (current_window_index > 0)
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

            case KEY_F(9): {
                touch(QString(getenv("HOME")) + "/.shutdown");
                status = "Terminating ServiceSpawner (services remain in background)";
                } break;

            case 10: /* TODO: implement details view */ {
                status = "Not implemented";
                } break;

            case 'A': /* Autotart */ {
                    if (APPS_NUMBER == 0) {
                        status = "Can't set autostart for non existant service";
                        break;
                    }
                    QString autostartFile = cursorAppDataDir + DEFAULT_SERVICE_AUTOSTART_FILE;
                    if (not QFile::exists(autostartFile))
                        touch(autostartFile);
                    else
                        QFile::remove(autostartFile);
                }
                status = "Triggered autostart of application: " + cursorBaseDir.baseName();
                break;

            case 'S': /* Start */
                if (APPS_NUMBER == 0) {
                    status = "Can't start non existant service";
                    break;
                }
                touch(cursorAppDataDir + "/.start");
                status = "Triggered start of application: " + cursorBaseDir.baseName();
                break;

            case 'T': /* sTop */
                if (APPS_NUMBER == 0) {
                    status = "Can't stop non existant service";
                    break;
                }
                touch(cursorAppDataDir + "/.stop");
                status = "Triggered stop of application: " + cursorBaseDir.baseName();
                break;

            case 'V': /* Validate */
                if (APPS_NUMBER == 0) {
                    status = "Can't validate non existant service";
                    break;
                }
                touch(cursorAppDataDir + "/.validate");
                status = "Triggered validation of application: " + cursorBaseDir.baseName();
                break;

            case 'I': /* Install */
                if (APPS_NUMBER == 0) {
                    status = "Can't install non existant service";
                    break;
                }
                touch(cursorAppDataDir + "/.install");
                status = "Triggered installation of application: " + cursorBaseDir.baseName();
                break;

            case 'C': /* Configure */
                if (APPS_NUMBER == 0) {
                    status = "Can't configure non existant service";
                    break;
                }
                touch(cursorAppDataDir + "/.configure");
                status = "Triggered configuration of application: " + cursorBaseDir.baseName();
                break;

            case 'R': /* Restart */
                if (APPS_NUMBER == 0) {
                    status = "Can't restart non existant service";
                    break;
                }
                touch(cursorAppDataDir + "/.restart");
                status = "Triggered restart of application: " + cursorBaseDir.baseName();
                break;

        }

        while (!kbhit()) {
            /* reload services list */
            apps = getApps(home);
            APPS_NUMBER = apps.length();

            cursorBaseDir = apps.at(current_window_index);
            cursorAppDataDir = cursorBaseDir.absolutePath() + "/" + cursorBaseDir.baseName();

            /* create log window, and show it on right side */
            QString outputFile = cursorAppDataDir + DEFAULT_SERVICE_LOG_FILE;
            WINDOW *win = newwin(row - row/2 + 10, col - 20 - 90, 2, 95);

            /* render log window */
            QString contents = tail(outputFile, row/2 + 8, modifier);
            wattron(win, COLOR_PAIR(6));
            box(win, 1, 1);
            mvwprintw(win, 0, 2, ("Log source: " + cursorBaseDir.path() + "/" + cursorBaseDir.baseName() + DEFAULT_SERVICE_LOG_FILE).toUtf8());
            wattroff(win, COLOR_PAIR(6));
            if (not contents.trimmed().isEmpty())
                mvwprintw(win, 1, 1, contents.trimmed().toUtf8());
            wrefresh(win);

            /* write app header */
            attron(COLOR_PAIR(1));
            QString info = "Conrol Panel, version: " + QString(APP_VERSION) + ". " + QString(COPYRIGHT);
            mvprintw(0, 1, info.toUtf8());

            /* services count info */
            attron(COLOR_PAIR(5));
            mvprintw(0, 88, "Defined services count: " + QString::number(APPS_NUMBER).toUtf8());
            attroff(COLOR_PAIR(5));

            /* SS status info */
            updateSSStatus(); /* will show status of service spawner */

            mvprintw(3, 0, " # Name                        PID Address                Status        Flags    Autostart");
            attroff(COLOR_PAIR(1));

            char flags[6];
            flags[5] = '\0';


            for (int i = 0; i < max_rows; i++) {
                int app_i = app_index_first + i;

                QFileInfo baseDir = apps.at(app_i);
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
                if (domain.isEmpty()) domain = "-";
                if (port.isEmpty()) port = "-";

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

                if(current_window_index == app_i){
                    attron(COLOR_PAIR(4));
                } else {
                    attron(color);
                }

                /* # */
                mvprintw(y, x, "%2d", app_i+1);
                x += 2;

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
            delwin(win);
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
