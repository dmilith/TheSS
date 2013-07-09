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
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));

    QString logFile = QString(getenv("HOME")) + CONTROL_PANEL_LOG_FILE;
    // QFile::remove(logFile);

    // Logger setup - turn it off
    FileAppender *consoleAppender = new FileAppender(logFile);
    Logger::registerAppender(consoleAppender);
    consoleAppender->setDetailsLevel(Logger::Debug);

    // Panel setup
    QString user = getenv("USER");
    QString home = getenv("HOME");
    QDir homeDir(home);
    QString ignitersPrefix = "/Igniters/Services";
    QDir ignitersDir(home + ignitersPrefix);
    if (getuid() == 0) { /* case when super user is running panel */
        homeDir = QDir(QString(SYSTEMUSERS_HOME_DIR));
        ignitersDir = QDir(SYSTEMUSERS_HOME_DIR + ignitersPrefix);
    }


    Panel * panel = new Panel(user, homeDir, ignitersDir);
    gui = new PanelGui(panel);

    // QThread * guiThread = new QThread();
    // gui->moveToThread(guiThread);
    // app.connect(guiThread, SIGNAL(started()), gui, SLOT(run()));
    app.connect(gui, SIGNAL(quit()), &app, SLOT(quit()));

    // panel->start();
    // gui->run();
    // guiThread->start();

    // gui->display();

    QTimer *timer1 = new QTimer(&app);
    app.connect(timer1, SIGNAL(timeout()), panel, SLOT(refresh()));
    timer1->start(100);

    gui->init();

    QTimer *timer2 = new QTimer(&app);
    app.connect(timer2, SIGNAL(timeout()), gui, SLOT(readInput()));
    timer2->start(10);

    QTimer *timerNotification = new QTimer(&app);
    app.connect(timerNotification, SIGNAL(timeout()), gui, SLOT(gatherNotifications()));
    timerNotification->start(100);

    return app.exec();
}
