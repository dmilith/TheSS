/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2013 - VerKnowSys
*/


#include "dispel.h"
#include "dispel_core.h"
#include "dispel_publisher.h"
#include "dispel_subscriber.h"


/*
    example value for knownNodes file:

    {
      "knownNodes": [
          ["127.0.0.1", "{40a8e817-57de-4e5e-9806-2c1338a5079b}"],
          ["192.168.0.9", "{40a8e817-57de-4e5e-9806-2c1338a5079b}"]
      ]
    }

*/


QMap<QString, QString> readNodesData() {
    auto map = QMap<QString, QString>();
    if (getuid() == 0) {
        QMap<QString,QString> nodes = allKnownNodeUUIDs();
        logInfo() << "Known nodes:" << nodes.values();
        Q_FOREACH(QString node, nodes) {
            map.insert(DEFAULT_LOCAL_ADDRESS, node);
        }
    } else {
        map.insert("192.168.0.9", "{devel-node-uuid}"); // XXX: local box
        map.insert("192.168.0.10", "{devel-node-uuid}"); // XXX: local vm
    }
    return map;
}


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));

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

    /* Logger setup */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    assert(consoleAppender);
    Logger::registerAppender(consoleAppender);
    consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c:(%F:%i)> %m\n");
    if (trace && debug)
        consoleAppender->setDetailsLevel(Logger::Trace);
    else if (debug && !trace)
        consoleAppender->setDetailsLevel(Logger::Debug);
    else {
        consoleAppender->setDetailsLevel(Logger::Debug); // INFO!
        consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] %m\n");
    }

    uint uid = getuid();
    switch (uid) {
        case 0: {
            logInfo() << "Checking existance and access priviledges of ServeD private directories";
            // readOrGenerateNodeUuid();
            if (not QDir().exists(DISPEL_NODE_KNOWN_NODES_DIR))
                QDir().mkpath(DISPEL_NODE_KNOWN_NODES_DIR);
            chmod(DISPEL_NODE_IDENTIFICATION_FILE, 0600); /* rw------- */
            chmod(DISPEL_NODE_KNOWN_NODES_DIR, 0600); /* rw------- */

        } break;

        default:
            logError() << "Please note that this software requires super user priviledges to run!";
            // logFatal()
    }

    logInfo("The ServeD Dispel v" + QString(APP_VERSION) + ". " + QString(COPYRIGHT));
    logInfo("Using Zeromq v" + zmqVersion());

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

