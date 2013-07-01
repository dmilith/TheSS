/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

 #include "panel.h"

void PanelGui::init(){
    // remove ESC key delay
    if (getenv ("ESCDELAY") == NULL)
        ESCDELAY = 0;

    // ncurses init crap
    initscr();
    getmaxyx(stdscr, rows, cols); // get cols and rows amount
    cbreak();
    start_color();
    curs_set(0); // cursor invisible
    keypad(stdscr, TRUE);
    noecho();
    refresh();

    // /* selected color */
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // default
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // running service
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // stopped service
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // selected service
    init_pair(5, COLOR_CYAN, COLOR_BLACK); // status
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);

    servicesList = new ServicesList(rows - 6);
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
    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "Control Panel v%s. © 2013 verknowsys.com", APP_VERSION);
}

void PanelGui::displayFooter(){
    QList<QString> actions, functions;

    actions << "Autostart" << "Start" << "sTop" << "Restart";
    actions << "Validate" << "Install" << "Configure" << "New" << "eXplode";
    functions << "F1" << "trace" << "F2" << "debug";
    functions << "F3" << "info" << "F4" << "error";
    functions << "F5" << "refresh" << "F7" << "new service";
    functions << "F8" << "destroy" << "F9" << "SS shutdown";

    int x = 0, y = rows - 2;
    char * str;
    int i;

    mvprintw(y, x, "Services: ");
    x+=10;
    Q_FOREACH(auto action, actions){
        str = action.toUtf8().data();
        for(i=0; i<action.length(); i++){
            if(str[i] >= 'A' && str[i] <= 'Z') attron(C_GREEN);
            else attron(C_DEFAULT);
            mvprintw(y, x++, "%c", str[i]);
        }
        mvprintw(y, x++, " ");
    }

    attron(C_GREEN);
    mvprintw(y, x++, "?");
    attron(C_DEFAULT);
    mvprintw(y, x++, "=Help");

    y++;
    x=0;

    for(i=0; i<functions.length()/2; i++){
        QString a = functions.at(2*i);
        QString b = functions.at(2*i+1);

        attron(C_STATUS);
        mvprintw(y, x, "%s", a.toUtf8().data());
        x+=a.length();
        attron(C_DEFAULT);
        mvprintw(y, x, "=%s ", b.toUtf8().data());
        x+=b.length()+2;
    }
}

void PanelGui::displayStatus(){
    /* services count info */
    // attron(COLOR_PAIR(5));
    // mvprintw(0, 88, "Defined services count: " + QString::number(panel->services.length()).toUtf8());
    // attroff(COLOR_PAIR(5));

    /* SS status info */
    if(panel->isSSOnline()){
        attron(C_SS_STATUS_ON);
        mvprintw(0, 45, ("ServiceSpawner: ONLINE  (" + QHostInfo::localHostName() + ")").toUtf8());
        attroff(C_SS_STATUS_ON);
    } else {
        attron(C_SS_STATUS_OFF);
        mvprintw(0, 45, ("ServiceSpawner: OFFLINE (" + QHostInfo::localHostName() + ")").toUtf8());
        attroff(C_SS_STATUS_OFF);
    }

    attron(C_STATUS);
    mvprintw(1, 0, "status: %-130s", status.toUtf8().data()); // XXX: hardcoded max length of status content
    attroff(C_STATUS);
    move(2, 0);
    clrtoeol();
}

void tmux(QString cmd){
    if(TMUX){
        cmd = " tmux send-keys -t 1 " + cmd + "\n";
        auto process = new SvdProcess("tail", getuid(), false);
        process->spawnProcess(cmd);
        process->waitForFinished();
    }
}

void PanelGui::displayFile(QString file){
    QString tpl = ":e %1 C-m";

    tmux("C-c");
    tmux(tpl.arg(file));

    if(QFile::exists(file)) tmux("F");
}

void PanelGui::displayLog(){
    const PanelService * service = servicesList->currentItem();
    if(TMUX && service != NULL && (loggedServicePath != service->dir.absolutePath())){
        loggedServicePath = service->dir.absolutePath();

        displayFile(loggedServicePath + DEFAULT_SERVICE_LOG_FILE);
    }
}

void PanelGui::displayConfig(){
    const PanelService * service = servicesList->currentItem();
    if(service != NULL){
        displayFile(service->dir.absolutePath() + "/service.conf");
    }
}

void PanelGui::displayEnv(){
    const PanelService * service = servicesList->currentItem();
    if(service != NULL){
        displayFile(service->dir.absolutePath() + "/service.env");
    }
}

void PanelGui::cleanup(){
    endwin();

    if(TMUX){
        auto process = new SvdProcess("tail", getuid(), false);
        process->spawnProcess(" tmux kill-session");
        process->waitForFinished(60);
    }
}

void PanelGui::searchLog(){
    tmux("C-c");
    tmux("/");
    QString cmd = " tmux select-pane -t 1\n";
    auto process = new SvdProcess("tail", getuid(), false);
    process->spawnProcess(cmd);
    process->waitForFinished();
}

void PanelGui::helpDialog(){
    WINDOW *win = newwin(rows-4, cols-2, 2, 1);

    wattron(win, COLOR_PAIR(6));
    box(win, '|', '-');
    wattroff(win, COLOR_PAIR(6));

    QList<QString> list;
    list << COPYRIGHT;
    list << "";
    list << "Available key bindings";
    list << "";
    list << "Current service actions:";
    list << "  S       - Start current service";
    list << "  D       - Change domain of current service";
    list << "  T       - Stop current service";
    list << "  R       - Restart current service";
    list << "  O       - Reload current service";
    list << "  I       - Install current service";
    list << "  C       - Configure current service";
    list << "  F       - Reconfigure current service";
    list << "  V       - Validate current service";
    list << "  A       - Toggle autostart";
    list << "  F8, X   - Delete current service";
    list << "  K       - Show app config (service.conf)";
    list << "  E       - Show app env (service.env)";
    list << "  L       - Refresh log pane (service.log)";
    list << "";
    list << "Log window actions:";
    list << "  [, ', ;, \\ - Move in log window";
    list << "  W       - Toggle line wrap";
    list << "";
    list << "Other actions:";
    list << "  F1      - Set trace log level";
    list << "  F2      - Set debug log level";
    list << "  F3      - Set info log level";
    list << "  F4      - Set error log level";
    list << "  F5      - Refresh panel";
    list << "  F6      - Display thess log";
    list << "  F7, N   - Add new service";
    list << "  F9      - Shutdown TheSS ";

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
                    QString selected = *list.currentItem();
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

    wattron(win, COLOR_PAIR(8));
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');

    WINDOW *inner = derwin(win, 3, c-2, 1, 1);

    mvwprintw(inner, 0, 0, (msg + " (Y/y/↵ to confirm)").toUtf8().data());
    wattron(inner, COLOR_PAIR(2));
    wrefresh(win);
    int ch = wgetch(win);

    werase(inner);
    werase(win);
    wrefresh(inner);
    wrefresh(win);
    delwin(inner);
    delwin(win);
    return (ch == 10 || ch == 'y' || ch == 'Y');
}

void PanelGui::key(int ch){
    switch(ch){
        case 'q':
            // just exit
            break;

        case KEY_UP:
        case KEY_DOWN:
            servicesList->key(ch);
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

        case KEY_F(6): /* display thess log */
            displayFile(panel->home.absoluteFilePath(".thess.log"));
            break;

        case KEY_F(9):
            panel->shutdown();
            status = "Terminating ServiceSpawner (services remain in background)";
            break;

        case '?':
            helpDialog();
            break;

        case '/':
            searchLog();
            break;

        case KEY_PPAGE:
        case '[':
            tmux("C-c");
            tmux("u");
            break;

        case KEY_NPAGE:
        case '\'':
            tmux("C-c");
            tmux("d");
            break;

        case ';':
            tmux("C-c");
            tmux("Escape \\(");
            break;

        case '\\':
            tmux("C-c");
            tmux("Escape \\)");
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

        case 'W': /* Toggle wrap on most */
            wrapLines = !wrapLines;
            tmux("C-c -S C-m");
            tmux("F C-m");
            break;

        case 'K':
            displayConfig();
            break;

        case 'E':
            displayEnv();
            break;

        case 'L': // refresh log window
            loggedServicePath = "";
            displayLog();
            break;

        case 10: /* TODO: implement details view */
            status = "Not implemented";
            break;

        case 'N':
        case KEY_F(7): /* Launch new service */
            newServiceDialog();
            break;

        case 'X':
        case KEY_F(8): /* Remove current service */
            removeCurrentService();
            break;

    }
}

void PanelGui::run(){
    init();

    int ch = 0;
    while(ch != 'q'){
        while(!kbhit()){
            panel->refreshServicesList();
            servicesList->setItems(&panel->services);

            if(panel->services.length() == 0){
                status = "No initialized services found in data directory. Hit F7 to add new.";
            }

            displayHeader();
            displayStatus();
            servicesList->display();
            displayFooter();

            refresh();
            displayLog();

            usleep(DEFAULT_PANEL_REFRESH_INTERVAL / 3);
            standend();
        }

        ch = getch();
        key(ch);
    }

    cleanup();
}
