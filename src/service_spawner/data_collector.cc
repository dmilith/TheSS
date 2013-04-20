/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "data_collector.h"


SvdDataCollector::SvdDataCollector() {
    /* launch new collector service */
    auto config = new SvdServiceConfig(name);

    /* setup connection to service */
    socketFile = config->prefixDir() + DATA_COLLECTOR_SOCKET_FILE;
    triggerFile = getHomeDir() + DEFAULT_SS_PROCESS_DATA_COLLECTION_HOOK_FILE;

    collector = new QTimer(this);
    connect(collector, SIGNAL(timeout()), this, SLOT(collectorGatherSlot()));
    collector->start(COLLECTOR_TIMEOUT_INTERVAL / 1000); // miliseconds

    logDebug() << "Launched stats collector";
    delete config;
}


void SvdDataCollector::connectToDataStore() {

    if (not QFile::exists(socketFile)) {
        logInfo() << "Detected issue with non existant socket file…";

        /* restart collector service cause something went wrong */
        auto config = new SvdServiceConfig(name);
        QFile::remove(config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE);
        touch(config->prefixDir() + "/.restart");
        delete config;

    } else {

        logInfo() << "Connecting to data collector @:" << socketFile;
        context = redisConnectUnix(socketFile.toUtf8());
        if (context == NULL || context->err) {
            if (context) {
                logFatal() << "Connection error:" << context->errstr;
                redisFree(context);
            } else {
                logFatal() << "Connection error: can't allocate redis context";
            }
            connected = false;
            return;
        }

        connected = true;
        logDebug() << "Connected to stats collector";
    }
}


void SvdDataCollector::collectorGatherSlot() {
    if (QFile::exists(triggerFile)) {
        if (not connected) {
            logInfo() << "Data storage not connected. Reconnecting.";
            connectToDataStore();
        }

        /* initialize random seed */
        srand(time(NULL));

        /* perform data storage query: */
        redisReply *reply = (redisReply*)redisCommand(context, "HMSET procname-%d:%d:%d cpu 20 rss 55 ioin 12 iout 35", rand() % 24, rand() % 60, rand() % 60); // XXX: hardcoded PoC
        logTrace() << "HMSET REPLY:" << reply->str;
        freeReplyObject(reply);
    }
}
