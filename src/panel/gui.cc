/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "gui.h"
#include <time.h>

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

    mainWindow = newwin(rows - 6, cols/2, 0, 0);
    logWindow = newwin(rows, cols/2, 0, cols/2);
    notificationWindow = newwin(notificationRows, cols/2, rows - notificationRows - 1, 0);
    wrefresh(notificationWindow);

    servicesList = new ServicesList(rows - 6, mainWindow);
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

                n.content = readFileContents(file.absoluteFilePath()).trimmed();
                n.time = file.created();
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
    mvwprintw(mainWindow, 0, 0, "Control Panel v%s. © 2013 verknowsys.com %d", APP_VERSION, time(NULL));
}

void PanelGui::displayFooter(){
    QList<QString> actions, functions;

    actions << "Autostart" << "Start" << "sTop" << "Restart";
    actions << "Validate" << "Install" << "Configure" << "New" << "eXplode";
    functions << "F1" << "trace" << "F2" << "debug";
    functions << "F3" << "info" << "F4" << "error";
    functions << "F5" << "refresh" << "F7" << "new service";
    functions << "F8" << "destroy" << "F9" << "SS shutdown";

    int x = 0, y = rows - 9;
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
        mvwprintw(mainWindow, 0, 45, ("ServiceSpawner: ONLINE  (" + QHostInfo::localHostName() + ")").toUtf8());
        wattroff(mainWindow, C_SS_STATUS_ON);
    } else {
        wattron(mainWindow, C_SS_STATUS_OFF);
        mvwprintw(mainWindow, 0, 45, ("ServiceSpawner: OFFLINE (" + QHostInfo::localHostName() + ")").toUtf8());
        wattroff(mainWindow, C_SS_STATUS_OFF);
    }

    wattron(mainWindow, C_STATUS);
    mvwprintw(mainWindow, 1, 0, "status: %-130s", status.toUtf8().data()); // XXX: hardcoded max length of status content
    wattroff(mainWindow, C_STATUS);
    wmove(mainWindow, 2, 0);
    wclrtoeol(mainWindow);
}


void PanelGui::cleanup(){
    endwin();
}


void PanelGui::helpDialog(){
    WINDOW *win = newwin(rows-4, cols-2, 2, 1);

    wattron(win, C_BORDER);
    box(win, '|', '-');
    wattroff(win, C_BORDER);

    QList<QString> list;
    list << "Available key bindings:";
    list << "";
    list << "Current service actions:";
    list << "  S       - Start current service";
    list << "  s       - Start current service without dependencies";
    list << "  D       - Change domain of current service";
    list << "  T       - Stop current service";
    list << "  t       - Stop current service without dependencies";
    list << "  R       - Restart current service";
    list << "  r       - Restart current service without dependencies";
    list << "  O       - Reload current service";
    list << "  I       - Install current service";
    list << "  C       - Configure current service";
    list << "  F       - Reconfigure current service";
    list << "  f       - Reconfigure current service without dependencies";
    list << "  V       - Validate current service";
    list << "  A       - Toggle autostart";
    list << "  F8, X   - Delete current service";
    list << "  K       - Show app config (service.conf)";
    list << "  E       - Show app env (service.env)";
    list << "  L       - Show service log (service.log)";
    list << "  W       - Toggle line wrapping in log window";
    list << "";
    list << "Other actions:";
    list << "  F1-F4   - Set log level (trace, debug, info, error)";
    list << "  F5      - Refresh panel";
    list << "  F7, N   - Add new service";
    list << "  F9      - Shutdown TheSS (leave services working)";

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


QString PanelGui::newDomain() {
    int r = min(rows-6, 23);
    WINDOW *win = newwin(r, 46, 2, 5);
    QList<QString> *aList = new QList<QString>();
    AvailableServicesList list(aList, r - 3, win, "   Domain name: ");
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

void PanelGui::key(int ch){
    switch(ch){
        case KEY_UP:
        case KEY_DOWN:
            servicesList->key(ch);
            displayLog();
            break;

        case 'D':
            if (servicesList->currentItem() == NULL) {
                status = "Can't change domain for non existant service";
            } else {
                QString name = servicesList->currentItem()->name;
                QString prefixPath = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + name;
                if (getuid() == 0) {
                    prefixPath = QString(SYSTEMUSERS_HOME_DIR) + SOFTWARE_DATA_DIR + "/" + name;
                }
                QString domainFilePath = prefixPath + QString(DEFAULT_SERVICE_DOMAIN_FILE);
                QString domain = newDomain();
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

        case KEY_F(9):
            panel->shutdown();
            status = "Terminating ServiceSpawner (services remain in background)";
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
}

void PanelGui::tailUpdate(){
    const PanelService * service = servicesList->currentItem();

    if(service != NULL){

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
    if(tail != NULL){
        tail->toggleWrap();
        tailUpdate();
    }
}

void PanelGui::tailReset(){
    if(tail != NULL){
        tail->resetScroll();
        tailUpdate();
    }
}

void PanelGui::tailScroll(int n){
    if(tail != NULL){
        tail->scrollLog(n, rows);
        tailUpdate();
    }
}

void PanelGui::displayConfig(){
    PanelService * service = servicesList->currentItem();
    if(service != NULL){
        tail = service->conf;
        tailUpdate();
    }
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

    // connect(this, SIGNAL(getInput()), this, SLOT(readInput())); // this will prevent blocking

    // emit display();
    // emit getInput();
}

void PanelGui::readInput(){
    // logDebug() << "readInput";
    if(kbhit()){
        int ch = getch();
        if(ch == 'q'){
            logDebug() << "cleanup";
            cleanup();
            emit quit();
            return;
        } else {
            key(ch);
            // emit display();
        }
    } else {
        // usleep(DEFAULT_PANEL_REFRESH_INTERVAL / 3);
    }

    // emit getInput();
}

