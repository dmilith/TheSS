/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2013 - VerKnowSys
*/


#include "dispel.h"
#include "dispel_core.h"
#include "dispel_publisher.h"
#include "dispel_subscriber.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    setDefaultEncoding();
    QCoreApplication::setOrganizationName("VerKnowSys");
    QCoreApplication::setOrganizationDomain("verknowsys.com");
    QCoreApplication::setApplicationName("ServeD");

    QStringList args = app.arguments();
    QRegExp rxEnableDebug("-d");
    QRegExp rxEnableTrace("-t");
    QRegExp rxPrintVersion("-v");

    bool debug = false, trace = false;
    for (int i = 1; i < args.size(); ++i) {
        if (rxEnableDebug.indexIn(args.at(i)) != -1 ) {
            debug = true;
        }
        if (rxEnableTrace.indexIn(args.at(i)) != -1 ) {
            debug = true;
            trace = true;
        }
        if (rxPrintVersion.indexIn(args.at(i)) != -1) {
            logInfo() << "ServeD Dispel v" << APP_VERSION << ". " << COPYRIGHT;
            return EXIT_SUCCESS;
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
    // } else {
    //     DestinationPtr fileDestination(
    //         DestinationFactory::MakeFileDestination(sLogPath, DisableLogRotation, MaxSizeBytes(512), MaxOldLogCount(0)));
    //     logger.addDestination(fileDestination);
    // }

    /* setting up file permissions */
    auto idFile = (getHomeDir() + DISPEL_NODE_IDENTIFICATION_FILE).toUtf8();
    chmod(idFile, 0600); /* rw------- */
    auto nodesFIle = (getHomeDir() + DISPEL_NODES_FILE).toUtf8();
    chmod(nodesFIle, 0600); /* rw------- */

    // uint uid = getuid();
    // switch (uid) {
    //     case 0: {

    //     } break;

    //     default:
    //         logWarn() << "Please note that this software requires super user priviledges to run!";
    //         // logFatal()
    // }

    logInfo() << "The ServeD Dispel v" << QString(APP_VERSION) << ". " << QString(COPYRIGHT);
    logInfo() << "Using Zeromq v" << zmqVersion();

    /* will create context with polling enabled by default */
    QScopedPointer<ZMQContext> context(createDefaultContext());
    assert(context);
    context->start();

    const QString nodeID = currentNodeUUID();

    /* launch local publisher */
    Publisher *publisher = new Publisher(*context, DISPEL_NODE_PUBLISHER_ENDPOINT, nodeID); /* use node id, as channel name */
    assert(publisher);
    publisher->start();

    /* read remote nodes list */
    auto subscribers = QList<Subscriber*>();

    const auto map = readNodesData();
    Q_FOREACH(auto ip, map.keys()) {
        QString nodeAddress = "tcp://" + ip + ":" + DISPEL_NODE_PUBLISHER_PORT;
        logInfo() << "Subscribing to Node ID:" << map.value(ip) << "->" << nodeAddress;

        Subscriber *subscriber = new Subscriber(*context, nodeAddress, map.value(ip)); /* use uuid, as channel name */
        assert(subscriber);
        subscriber->start();

        subscribers << subscriber;
    }

    logDebug() << "Subscribed Nodes:" << subscribers;
    return app.exec();
}

