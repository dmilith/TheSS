/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "panel.h"
#include "gui.h"
#include <signal.h>
#include <sys/ioctl.h>

PanelGui * gui;

void handle_winch(int sig){
    Q_UNUSED(sig);
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    gui->reload(w.ws_row, w.ws_col);
}


int main(int argc, char *argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);

    // App setup
    QCoreApplication app(argc, argv);
    setDefaultEncoding();
    QCoreApplication::setOrganizationName("VerKnowSys");
    QCoreApplication::setOrganizationDomain("verknowsys.com");
    QCoreApplication::setApplicationName("ServeD");
    QString logFile = QString(DEFAULT_HOME_DIR) + CONTROL_PANEL_LOG_FILE;

    // Logger setup - turn it off
    FileAppender *fileAppender = new FileAppender(logFile);
    Logger::registerAppender(fileAppender);
    fileAppender->setDetailsLevel(Logger::Warning);

    // Panel setup
    QString user = getenv("USER");
    QString home = DEFAULT_HOME_DIR;
    QDir homeDir(home);
    QString ignitersPrefix = "/Igniters/Services";
    QDir ignitersDir(home + ignitersPrefix);
    if (getuid() == 0) { /* case when super user is running panel */
        homeDir = QDir(QString(SYSTEM_USERS_DIR));
        ignitersDir = QDir(SYSTEM_USERS_DIR + ignitersPrefix);
    }

    Panel* panel = new Panel(user, homeDir, ignitersDir);
    gui = new PanelGui(panel);
    gui->init();
    gui->display();

    QTimer *timer1 = new QTimer(&app);
    app.connect(timer1, SIGNAL(timeout()), panel, SLOT(refresh()));
    app.connect(timer1, SIGNAL(timeout()), gui, SLOT(readInput()));
    app.connect(timer1, SIGNAL(timeout()), gui, SLOT(gatherNotifications()));
    timer1->start(DEFAULT_PANEL_TIMER_PAUSE);

    app.connect(gui, SIGNAL(quit()), &app, SLOT(quit()));
    return app.exec();
}
