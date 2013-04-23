/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "death_watch.h"


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    QStringList args = app.arguments();

    /* Logger setup */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
    bool debug = true, trace = true;
    if (trace && debug)
        consoleAppender->setDetailsLevel(Logger::Trace);
    else if (debug && !trace)
        consoleAppender->setDetailsLevel(Logger::Debug);
    else {
        consoleAppender->setDetailsLevel(Logger::Info);
        consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
    }

    /* check args */
    if (args.size() == 1) {
        logError() << "You need to specify pid to put a death watch on.";
        return 1;
    }
    bool ok = false;
    pid_t pid = args.at(1).toInt(&ok, 10);
    if (ok) {
        logInfo() << "Death watch is in harvest mode for pid:" << QString::number(pid);
        deathWatch(pid);
        return 0;
    }

    // return app.exec();
}
