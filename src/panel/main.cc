/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "panel.h"
#include <signal.h>
#include <sys/ioctl.h>

bool TMUX;

Panel * panel;
PanelGui * gui;

void handle_winch(int sig){
    Q_UNUSED(sig);
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    gui->reload(w.ws_row, w.ws_col);
}

int main(int argc, char *argv[]) {
    // Require to be run inside tmux
    TMUX = QString(getenv("TMUX_PANE")) != "";

    if(!TMUX){
        printf("Must be run inside tmux\n");
        exit(-1);
    }

    // App setup
    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));

    // Logger setup - turn it off
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setDetailsLevel(Logger::Fatal);

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

    panel = new Panel(user, homeDir, ignitersDir);
    gui = new PanelGui(panel);

    // handle window resize
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);

    gui->run();

    return 0;
}
