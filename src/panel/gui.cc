/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "gui.h"
#include <time.h>

static Tail * ssLog = NULL;
static bool firstRun = true;


void PanelGui::init(){
    // remove ESC key delay
    if (getenv ("ESCDELAY") == NULL)
        ESCDELAY = 0;

    // ncurses init crap
    initscr();
    getmaxyx(stdscr, rows, cols); // get cols and rows amount
    cbreak();
    start_color();
    ansi_setup();
    curs_set(0); // cursor invisible
    keypad(stdscr, TRUE);
    noecho();
    refresh();

    panel->setGui(this);

    const int modifier = 25;
    mainWindow = newwin(rows - notificationRows, cols/2 - modifier, 0, 0);
    logWindow = newwin(rows, cols/2 + modifier, 0, cols/2 - modifier);
    notificationWindow = newwin(notificationRows, cols/2 - modifier, rows - notificationRows, 0);
    wrefresh(notificationWindow);

    servicesList = new ServicesList(rows - notificationRows, mainWindow);
    servicesList->setItems(&panel->services);

    if(panel->services.length() == 0){
        status = "No initialized services found in data directory. Hit F7 to add new.";
    }
}


bool NotificationLessThan(const Notification &a, const Notification &b){
    return a.time < b.time;
}


void PanelGui::gatherNotifications() {
    int i = 0, x = 0;
    QString userSoftwarePrefix = QString(getenv("HOME")) + QString(SOFTWARE_DATA_DIR);
    if (getuid() == 0) {
        userSoftwarePrefix = QString(SYSTEM_USERS_DIR) + QString(SOFTWARE_DATA_DIR);
    }
    auto userSoftwareList = QDir(userSoftwarePrefix).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QList<Notification> notifications;

    /* iterate through user software to find software notifications */
    Q_FOREACH(QString service, userSoftwareList) {
        QString notificationsPrefix = userSoftwarePrefix + NOTIFICATIONS_DATA_DIR;
        if (QDir().exists(notificationsPrefix)) {
            auto files = QDir(notificationsPrefix).entryInfoList(QDir::Files, QDir::Time);

            Q_FOREACH(auto file, files) {
                Notification n;
                QString ext = file.suffix();

                if (ext == "error")         n.level = NOTIFICATION_LEVEL_ERROR;
                else if (ext == "warning")  n.level = NOTIFICATION_LEVEL_WARNING;
                else if(ext == "notice")    n.level = NOTIFICATION_LEVEL_NOTICE;

                n.content = readFileContents(file.absoluteFilePath()).trimmed();
                n.time = file.created().toMSecsSinceEpoch();

                bool allow = true;
                Q_FOREACH(Notification an, notifications) {
                    if (an.content.trimmed() == n.content.trimmed())
                        allow = false;
                }
                if (allow)
                    notifications.append(n);
            }
        }
    }

    // Sort
    qSort(notifications.begin(), notifications.end(), NotificationLessThan);

    // Display
    int s = notifications.size();
    int start = max(0, s - notificationRows);
    int stop = min(s, notificationRows);

    logTrace() << "s: " << s << " start: " << start << " stop: " << stop;

    wattron(notificationWindow, C_DEFAULT);
    wmove(notificationWindow, 0, 0);
    whline(notificationWindow, '-', cols/2);

    for(; i<stop; i++){
        Notification n = notifications.at(i+start);
        switch(n.level){
            case NOTIFICATION_LEVEL_ERROR:
                wattron(notificationWindow, C_NOTIFICATION_ERROR);
                break;
            case NOTIFICATION_LEVEL_WARNING:
                wattron(notificationWindow, C_NOTIFICATION_WARNING);
                break;
            case NOTIFICATION_LEVEL_NOTICE:
                wattron(notificationWindow, C_NOTIFICATION_NOTICE);
                break;
        }

        mvwprintw(notificationWindow, i+1, x, n.content.toUtf8());
        wclrtoeol(notificationWindow);
    }
    for(; i<rows;i++){
        wmove(notificationWindow, i+1, x);
        wclrtoeol(notificationWindow);
    }

    wrefresh(notificationWindow);
    refresh();
}


int PanelGui::kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void PanelGui::displayHeader(){
    wattron(mainWindow, C_DEFAULT);
    mvwprintw(mainWindow, 0, 0, "Control Panel v%s. © 2013 verknowsys.com", APP_VERSION);
}

void PanelGui::displayFooter(){
    QList<QString> actions, functions;

    actions << "Autostart" << "Start" << "sTop" << "Restart"
            << "Validate" << "Install" << "Configure" << "New" << "eXplode";

    functions << "F1" << "trc" << "F2" << "dbg"
              << "F3" << "inf" << "F4" << "err"
              << "F5" << "rfsh" << "F6" << "rnm" << "F7" << "new"
              << "F8" << "dstr" << "F9" << "forced ss on/off" << "F10" << "ss on/off";

    int x = 0, y = rows - 15;
    char * str;
    int i;

    mvwprintw(mainWindow, y, x, "Services: ");
    x+=10;
    Q_FOREACH(auto action, actions){
        str = action.toUtf8().data();
        for(i=0; i<action.length(); i++){
            if(str[i] >= 'A' && str[i] <= 'Z') wattron(mainWindow, C_GREEN);
            else wattron(mainWindow, C_DEFAULT);
            mvwprintw(mainWindow, y, x++, "%c", str[i]);
        }
        mvwprintw(mainWindow, y, x++, " ");
    }

    wattron(mainWindow, C_GREEN);
    mvwprintw(mainWindow, y, x++, "?");
    wattron(mainWindow, C_DEFAULT);
    mvwprintw(mainWindow, y, x++, "=Help");

    y++;
    x=0;

    for(i=0; i<functions.length()/2; i++){
        QString a = functions.at(2*i);
        QString b = functions.at(2*i+1);

        wattron(mainWindow, C_STATUS);
        mvwprintw(mainWindow, y, x, "%s", a.toUtf8().data());
        x+=a.length();
        wattron(mainWindow, C_DEFAULT);
        mvwprintw(mainWindow, y, x, "=%s ", b.toUtf8().data());
        x+=b.length()+2;
    }
}

void PanelGui::displayStatus(){
    /* SS status info */
    if(panel->isSSOnline()){
        wattron(mainWindow, C_SS_STATUS_ON);
        mvwprintw(mainWindow, 0, 48, ("svdss:  ONLINE (" + QHostInfo::localHostName() + ")").toUtf8());
        wattroff(mainWindow, C_SS_STATUS_ON);
    } else {
        wattron(mainWindow, C_SS_STATUS_OFF);
        mvwprintw(mainWindow, 0, 48, ("svdss: OFFLINE (" + QHostInfo::localHostName() + ")").toUtf8());
        wattroff(mainWindow, C_SS_STATUS_OFF);
    }

    wattron(mainWindow, C_STATUS);
    mvwprintw(mainWindow, 1, 0, "status: %-130s", status.toUtf8().data()); // XXX: hardcoded max length of status content
    wattroff(mainWindow, C_STATUS);
    wmove(mainWindow, 2, 0);
    wclrtoeol(mainWindow);
}


void PanelGui::cleanup(){
}


void PanelGui::helpDialog(){
    WINDOW *win = newwin(rows-4, cols-2, 2, 1);

    wattron(win, C_BORDER);
    box(win, '|', '-');
    wattroff(win, C_BORDER);

    QList<QString> list;
    list << "Available key bindings:";
    list << "Current service actions:";
    list << "  S / s   - Start current service / Start current service without dependencies";
    list << "  T / t   - Stop current service / Stop current service without dependencies";
    list << "  F / f   - Reconfigure current service / Reconfigure current service without dependencies";
    list << "  R / r   - Restart current service / Restart current service without dependencies";
    list << "  D       - Change domain of current service";
    list << "  O       - Reload current service";
    list << "  I       - Install current service";
    list << "  C       - Configure current service";
    list << "  V       - Validate current service";
    list << "  A       - Toggle autostart";
    list << "  K       - Show app config (service.conf)";
    list << "  E       - Show app env (service.env)";
    list << "  L       - Show service log (service.log)";
    list << "  W       - Toggle line wrapping in log window";
    list << "  ↵       - Edit service igniter in vim editor";
    list << "Other actions:";
    list << "  F1-F4   - Set log level (trace, debug, info, error)";
    list << "  F5      - Refresh panel";
    list << "  F6      - Rename service (also creates duplicate of current igniter)";
    list << "  F7, N   - Add new service";
    list << "  F8, X   - Delete current service";
    list << "  F9      - Launch TheSS, or shutdown TheSS if running (services stay)";
    list << "  F10     - Launch TheSS, or gracefully shutdown TheSS if running";
    list << "  F11     - Forced TheSS shutdown (make it dead, no matter what)";
    list << "  F12     - Reset service state & trigger files (useful for igniter debugging)";
    list << "  ` ~ \\   - Show TheSS log";

    for(int i=0; i<list.length(); i++){
        mvwprintw(win, i+1, 2, "%s", list.at(i).toUtf8().data());
    }

    wgetch(win);
    werase(win);
    wrefresh(win);
    delwin(win);
}

void PanelGui::newServiceDialog(){
    int r = min(rows-6, 23);
    WINDOW *win = newwin(r, 46, 2, 5);
    AvailableServicesList list(panel->availableServices(), r - 3, win);
    QString name = "";
    int ch = 0;

    keypad(win, TRUE);
    curs_set(1); // cursor visible

    do {
        list.display();
        ch = wgetch(win);

        switch(ch){
            case KEY_UP:
            case KEY_DOWN:
                list.key(ch);
                break;

            case 10: // enter
                {
                    QString selected = list.currentItem();
                    status = panel->addService(selected);
                    servicesList->setItems(&panel->services);
                    servicesList->setCurrent(selected);
                    ch = 27;
                }
                break;

            case 127: // backspace
                name.truncate(name.length()-1);
                list.setName(name);
                break;

            default:
                name += ch;
                list.setName(name);
                break;
        }
    } while(ch != 27);

    curs_set(0);
}


QString PanelGui::newEntry(QString defaultEntry = "Domain name") {
    int r = min(rows-6, 23);
    WINDOW *win = newwin(r, 46, 2, 5);
    QList<QString> *aList = new QList<QString>();
    AvailableServicesList list(aList, r - 3, win, "  " + defaultEntry + ": ");
    QString name = "";
    int ch = 0;

    keypad(win, TRUE);
    curs_set(1); // cursor visible

    do {
        list.display();
        ch = wgetch(win);

        switch(ch){

            case 27: // escape
                name = "";
                break;

            case 10: // enter
                ch = 27;
                break;

            case 127: // backspace
                name.truncate(name.length()-1);
                list.setName(name);
                break;

            default:
                name += ch;
                list.setName(name);
                break;
        }
    } while (ch != 27);

    delete aList;
    curs_set(0);
    return name;
}


void PanelGui::removeCurrentService(){
    auto service = servicesList->currentItem();
    if(service == NULL){
        status = "You can't remove non existant service.";
    } else {
        if(service->isRunning){
            status = "You can't remove running service: " + service->name;
        } else {
            if(confirm(" Are you sure you want to destroy data\n and configuration of service: " + service->name + "?")){
                if(service->remove()){
                    status = "Data dir removed: " + service->dir.absolutePath();
                } else {
                    status = "Permissions failure. Check your user priviledges on your software data dir";
                }
            } else {
                status = "Cancelled removing data dir: " + service->name;
            }
        }
    }
}

void PanelGui::reload(int r, int c){
    rows = r;
    cols = c;
    servicesList->reset(rows - 6);
    clear();
    refresh();
    status = "Reloaded";
}

bool PanelGui::confirm(QString msg){
    int c = min(cols-2, 100);
    WINDOW *win = newwin(5, c, (rows-5)/2, (cols-c)/2);
    keypad(win, TRUE);

    wattron(win, C_NOTIFICATION_ERROR);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');

    WINDOW *inner = derwin(win, 3, c-2, 1, 1);

    mvwprintw(inner, 0, 0, (msg + " (Y to confirm)").toUtf8().data());
    wattron(inner, C_NOTIFICATION_NOTICE);
    wrefresh(win);
    int ch = wgetch(win);

    werase(inner);
    werase(win);
    wrefresh(inner);
    wrefresh(win);
    delwin(inner);
    delwin(win);
    return ch == 'Y';
}


QString launchSS() {
    uInt uid = getuid();
    auto prc = new SvdProcess("Thess", uid, false);
    #ifdef __FreeBSD__
        prc->spawnProcess("daemon svdss"); /* NOTE: it uses Sofin environment automatically */
    #else
        prc->spawnProcess("svdss &"); /* NOTE: it uses Sofin environment automatically */
    #endif
    prc->waitForFinished(1);
    notification("Launching ServiceSpawner", NOTIFY);
    return "Launching ServiceSpawner for " + QString(uid == 0 ? "SuperUser" : "NormalUser"); /* status msg */
}


void PanelGui::key(int ch){
    switch(ch){
        case KEY_UP:
        case KEY_DOWN:
            servicesList->key(ch);
            displayLog();
            break;

        case 10: { /* enter to the service prefix */
            servicesList->key(ch);
            endwin();
            QString name = servicesList->currentItem()->name;

            if (getuid() == 0)
                system(QString("vim " + QString(SYSTEM_USERS_DIR) + DEFAULT_USER_IGNITERS_DIR + "/" + name + DEFAULT_SOFTWARE_TEMPLATE_EXT).toUtf8().constData());
            else
                system(QString("vim " + QString(getenv("HOME")) + "/" + DEFAULT_USER_IGNITERS_DIR + "/" + name + DEFAULT_SOFTWARE_TEMPLATE_EXT).toUtf8().constData());

            initscr();
            status = "I've came back from the underground!";
        } break;

        case 'D':
            if (servicesList->currentItem() == NULL) {
                status = "Can't change domain for non existant service";
            } else {
                QString name = servicesList->currentItem()->name;
                QString prefixPath = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + name;
                if (getuid() == 0) {
                    prefixPath = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR + "/" + name;
                }
                QString domainFilePath = prefixPath + QString(DEFAULT_SERVICE_DOMAIN_FILE);
                QString domain = newEntry();
                status = "Changing domain for service: " + name + " to: " + domain;
                if (not domain.trimmed().isEmpty())
                    writeToFile(domainFilePath, domain);

            }
            break;

        case KEY_F(1): /* Trace */
            panel -> setLogLevel("trace");
            status = "Triggered log level change to: Trace";
            break;

        case KEY_F(2): /* Debug */
            panel->setLogLevel("debug");
            status = "Triggered log level change to: Debug";
            break;

        case KEY_F(3): /* Info */
            panel->setLogLevel("info");
            status = "Triggered log level change to: Info";
            break;

        case KEY_F(4): /* Warn */
            panel->setLogLevel("warn");
            status = "Triggered log level change to: Warning";
            break;

        case KEY_F(5): /* refresh */
            int r,c;
            getmaxyx(stdscr, r, c);
            reload(r, c);
            break;

        case KEY_F(6): /* rename service */
            if (servicesList->currentItem() == NULL) {
                status = "Can't change name of non existing service!";
            } else {
                QString name = servicesList->currentItem()->name;
                QString basePath = QString(getenv("HOME")) + SOFTWARE_DATA_DIR;
                QString prefixPath = basePath + "/" + name;
                if (getuid() == 0) {
                    basePath = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR;
                    prefixPath = basePath + "/" + name;
                }

                /* NOTE: stop service (without deps), rename directory, replace old abs paths in service.conf, start again if stopped */
                QString sname = newEntry("Service name");

                /* name validations */
                QString tmd = sname.trimmed();
                if (tmd == "") {
                    status = "Provided empty Service name. Aborted.";
                    break;
                }
                QList<QChar> forbiddens;
                forbiddens << '*' << '/' << '<' << '>' << '{' << '}' << '#' << '@' << '&' << '\\' << '"' << '%' << ';' << ':' << '[' << ']' << ' ';
                bool successful = true;
                Q_FOREACH(QChar forbidden, forbiddens)
                    if (tmd.indexOf(forbidden) != -1) {
                        status = "Forbidden characters given in Service name. Aborted.";
                        successful = false;
                        break;
                    }
                QString destPath = basePath + "/" + sname;

                if (successful) {
                    bool stpd = false;
                    auto item = servicesList->currentItem();
                    if (item->isRunning) {
                        item->stopWithoutDeps();
                        stpd = true;
                    }

                    /* replace paths in service.conf */
                    QString fileName = prefixPath + "/service.conf";
                    if (QFile::exists(fileName)) {
                        QString svcConf = readFileContents(fileName);
                        svcConf.replace(prefixPath, destPath);
                        writeToFile(fileName, svcConf);
                    }

                    /* add copy of igniter! */
                    QString igniterBaseDir = basePath + "/.." + DEFAULT_USER_IGNITERS_DIR;
                    QString igniter = igniterBaseDir + "/" + name + DEFAULT_SOFTWARE_TEMPLATE_EXT;
                    if (QFile::exists(igniter)) {
                        QFile::copy(igniter, igniterBaseDir + "/" + sname + DEFAULT_SOFTWARE_TEMPLATE_EXT);
                    }

                    copyPath(prefixPath, destPath);
                    removeDir(prefixPath);

                    if (stpd) { /* if service was stopped, start it again after rename */
                        touch(destPath + "/.startWithoutDeps");
                    }
                }
            }
            break;


        case KEY_F(9):
            if (panel->isSSOnline()) {
                status = "Terminating ServiceSpawner (services remain in background)";
                panel->shutdown();
            } else {
                status = launchSS();
            }
            break;


        case '\\':
        case '~':
        case '`': /* Show SS log */
            displaySSLog();
            break;


        case KEY_F(10):
            if (panel->isSSOnline()) {
                status = "Gracefully shutting down ServiceSpawner";
                panel->gracefullyTerminate();
            } else {
                status = launchSS();
            }
            break;


        case KEY_F(11):
            if (panel->isSSOnline()) {
                status = "Force ServiceSpawner shutdown";
                endwin();
                system("svddw $(cat $HOME/.$USER.pid)");
                initscr();
                refresh();
            }
            break;


        case KEY_F(12): {
                QString name = servicesList->currentItem()->name;
                status = "Stopping and resetting trigger and state files for service: " + name;

                auto config = new SvdServiceConfig(name);

                /* triggers */
                QFile::remove(config->prefixDir() + INSTALL_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + CONFIGURE_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + RECONFIGURE_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + RECONFIGURE_WITHOUT_DEPS_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + START_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + START_WITHOUT_DEPS_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + STOP_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + STOP_WITHOUT_DEPS_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + AFTERSTART_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + AFTERSTOP_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + RESTART_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + RELOAD_TRIGGER_FILE);
                QFile::remove(config->prefixDir() + VALIDATE_TRIGGER_FILE);

                /* states */
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_CRON_WORKING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_PID_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_INSTALLING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTOPPING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_AFTERSTARTING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_CONFIGURING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RELOADING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATING_FILE);
                QFile::remove(config->prefixDir() + DEFAULT_SERVICE_VALIDATION_FAILURE_FILE);

                delete config;
            }
            break;


        case '?':
            helpDialog();
            break;

        case KEY_PPAGE:
        case '[':
            tailScroll(5);
            break;

        case KEY_NPAGE:
        case '\'':
            tailScroll(-5);
            break;

        case 'S': /* Start */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't start non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->start();
                    status = "Triggered start of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 's': /* start without dependencies */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't start non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->startWithoutDeps();
                    status = "Triggered start of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'T': /* sTop */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't stop non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->stop();
                    status = "Triggered stop of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 't': /* stop without dependencies */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't stop non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->stopWithoutDeps();
                    status = "Triggered stop of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'V': /* Validate */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't validate non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->validate();
                    status = "Triggered validation of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'I': /* Install */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't install non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->install();
                    status = "Triggered installation of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'C': /* Configure */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't configure non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->configure();
                    status = "Triggered configuration of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'F': /* Reconfigure */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't reconfigure non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->reconfigure();
                    status = "Triggered reconfiguration of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'f': /* Reconfigure without dependencies */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't reconfigure non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->reconfigureWithoutDeps();
                    status = "Triggered reconfiguration of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'R': /* Restart */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't restart non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->restart();
                    status = "Triggered restart of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'r': /* restart without dependencies */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't restart non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->restartWithoutDeps();
                    status = "Triggered restart of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'O': /* RelOad */
            if (servicesList->currentItem() == NULL) {
                    status = "Can't reload non existant service";
            } else {
                if (panel->isSSOnline()) {
                    servicesList->currentItem()->reload();
                    status = "Triggered reload of application: " + servicesList->currentItem()->name;
                } else
                    status = "No Service Spawner running. Skipping task.";
            }
            break;

        case 'A': /* Autotart */
            if (servicesList->currentItem() == NULL) {
                status = "Can't set autostart for non existant service";
            } else {
                servicesList->currentItem()->toggleAutostart();
                status = "Triggered autostart of application: " + servicesList->currentItem()->name;
            }
            break;

        case 'W':
            tailToggleWrap();
            break;

        case 'K': /* show service config file */
            displayConfig();
            break;

        case 'E':
            displayEnv();
            break;

        case 'L': /* refresh log window */
            displayLog();
            break;

        case 'N':
        case KEY_F(7): /* Initialize new service */
            newServiceDialog();
            break;

        case 'X':
        case KEY_F(8): /* Destroy current service configuration with data */
            removeCurrentService();
            break;

    }
}

void PanelGui::display(){
    // logDebug() << "display";
    tailUpdate();
    displayHeader();
    displayStatus();
    servicesList->setItems(&panel->services);
    servicesList->display();
    displayFooter();
    wrefresh(mainWindow);
    standend();
    if (firstRun) {
        displaySSLog();
        firstRun = false;
    }
}

void PanelGui::tailUpdate(){
    const PanelService * service = servicesList->currentItem();

    if (service != NULL){

        if(service != recentService) {
            recentService = service;
            displayLog();
        }

        if(tail == NULL) tail = service->log;
        tail->display(logWindow, rows, cols);
    }

    wrefresh(logWindow);
}

void PanelGui::tailToggleWrap(){
    if (tail != NULL){
        tail->toggleWrap();
        tailUpdate();
    }
}

void PanelGui::tailReset(){
    if (tail != NULL){
        tail->resetScroll();
        tailUpdate();
    }
}

void PanelGui::tailScroll(int n){
    if (tail != NULL){
        tail->scrollLog(n, rows);
        tailUpdate();
    }
}

void PanelGui::displayConfig(){
    PanelService * service = servicesList->currentItem();
    if (service != NULL){
        tail = service->conf;
        tailUpdate();
    }
}

void PanelGui::displaySSLog(){
    PanelService * service = servicesList->currentItem();
    if (ssLog != NULL) {
        if (getuid() == 0)
            ssLog = new Tail(service, QString(SYSTEM_USERS_DIR), DEFAULT_SS_LOG_FILE);
        else
            ssLog = new Tail(service, QString(getenv("HOME")), DEFAULT_SS_LOG_FILE);
    }
    tail = ssLog;
    tailUpdate();
}

void PanelGui::displayEnv(){
    const PanelService * service = servicesList->currentItem();
    if(service != NULL){
        tail = service->env;
        tailUpdate();
    }
}

void PanelGui::displayLog(){
    const PanelService * service = servicesList->currentItem();
    if(service != NULL){
        tail = service->log;
        tailUpdate();
    }
}

void PanelGui::run(){
    init();
}

void PanelGui::readInput(){
    if(kbhit()){
        int ch = getch();
        if (ch == 'q'){
            endwin();
            exit(0);
        } else {
            key(ch);
        }
    } else {
        usleep(20000);
    }
}

