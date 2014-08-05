/**
 *  @author dmilith
 *
 *   © 2014 - VerKnowSys
 *
 */


#include "notifications.h"


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    setDefaultEncoding();
    QCoreApplication::setOrganizationName("VerKnowSys");
    QCoreApplication::setOrganizationDomain("verknowsys.com");
    QCoreApplication::setApplicationName("ServeD");
    QStringList args = app.arguments();
    QRegExp rxEnableDebug("-d");
    QRegExp rxLevel("-l");
    NotificationLevels logLevel = NOTIFY;

    /* Logger setup */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
    bool debug = false, trace = false;
    int pos = 0; /* position of argument */
    for (int i = 1; i < args.size(); ++i) {
        if (rxEnableDebug.indexIn(args.at(i)) != -1 ) {
            debug = true;
            pos++;
        }
        if (rxLevel.indexIn(args.at(i)) != -1 ) {
            logLevel = (NotificationLevels)args.at(i + 1).toInt();
            pos += 2;
        }
    }
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
        logInfo() << "You need to specify at least notification message as param.";
        return 1;
    }
    logDebug() << "Logger level:" << QString::number(logLevel);

    QStringList displayArgs;
    for (int i = 1 + pos; i < args.size(); ++i) {
        displayArgs << args.at(i);
    }
    notification(displayArgs.join(" "), logLevel);

    return 0;
}
