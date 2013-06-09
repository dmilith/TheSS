/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "panel.h"

bool TMUX;

int main(int argc, char *argv[]) {
    // Require to be run inside tmux
    TMUX = QString(getenv("TMUX")) != "";

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

    auto panel = new Panel(user, homeDir, ignitersDir);
    auto gui = new PanelGui(panel);
    gui->run();

    return 0;
}
