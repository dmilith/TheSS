/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "../core/death_watch.h"


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    setDefaultEncoding();
    QStringList args = app.arguments();
    QRegExp rxEnableDebug("-d");

    /* Logger setup */
    bool debug = false, trace = false;
    for (int i = 1; i < args.size(); ++i) {
        if (rxEnableDebug.indexIn(args.at(i)) != -1 ) {
            debug = true;
        }
    }

    using namespace QsLogging;
    Logger& logger = Logger::instance();
    const QString sLogPath(DEFAULT_SS_LOG_FILE);
    Level logLevel = InfoLevel;
    if (debug)
        logLevel = DebugLevel;
    if (trace)
        logLevel = TraceLevel;
    logger.setLoggingLevel(logLevel);

    /* Logger setup */
    DestinationPtr consoleDestination(DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(consoleDestination);

    /* check args */
    if (args.size() == 1) {
        logInfo() << "You need to specify pid to put a death watch on.";
        return 1;
    }
    logInfo() << "DeathWatch version" << APP_VERSION << COPYRIGHT;
    bool ok = false;
    pid_t pid = args.at(1).toInt(&ok, 10);
    if (ok) {
        logInfo() << "Death watch is harvesting soul of pid:" << QString::number(pid);
        deathWatch(pid);
    } else {
        QString possiblePidFile = args.at(1);
        if (QFile::exists(possiblePidFile)) { /* case when given param is name of file with pid, not pid itself */
            logInfo() << "Reading file:" << possiblePidFile;
            pid = readFileContents(possiblePidFile).trimmed().toInt(&ok, 10);
            if (ok) {
                logInfo() << "Death watch is harvesting soul of pid:" << QString::number(pid);
                deathWatch(pid);
            } else {
                logError() << "Bad pid file given:" << possiblePidFile << " Aborting.";
                return 1;
            }
        } else {
            logError() << "Bad pid number given:" << args.at(1) << " Aborting.";
            return 1;
        }
    }
    return 0;
}
