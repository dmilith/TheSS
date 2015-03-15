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
    // NotificationLevels logLevel = NOTIFY;

    /* Logger setup */
    bool debug = false, trace = false;
    int pos = 0; /* position of argument */
    for (int i = 1; i < args.size(); ++i) {
        if (rxEnableDebug.indexIn(args.at(i)) != -1 ) {
            debug = true;
            pos++;
        }
        if (rxLevel.indexIn(args.at(i)) != -1 ) {
            // logLevel = (NotificationLevels)args.at(i + 1).toInt();
            pos += 2;
        }
    }

    /* Logger setup */
    using namespace QsLogging;
    Logger& logger = Logger::instance();
    const QString sLogPath(DEFAULT_SS_LOG_FILE);
    Level logLevel = InfoLevel;
    if (debug)
        logLevel = DebugLevel;
    if (trace)
        logLevel = TraceLevel;
    logger.setLoggingLevel(logLevel);

    DestinationPtr consoleDestination(DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(consoleDestination);

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
    auto notificationLevel = NotificationLevels::NOTIFY;
    switch (logLevel) {
        case WarnLevel:
            notificationLevel = NotificationLevels::WARNING;
            break;
        case ErrorLevel:
            notificationLevel = NotificationLevels::ERROR;
            break;
        case FatalLevel:
            notificationLevel = NotificationLevels::FATAL;
            break;
        default:
            break;
    }
    notification(displayArgs.join(" "), notificationLevel);

    return 0;
}
