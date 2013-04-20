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

    collector = new QTimer(this);
    connect(collector, SIGNAL(timeout()), this, SLOT(collectorGatherSlot()));
    collector->start(COLLECTOR_TIMEOUT_INTERVAL / 1000); // miliseconds

    logDebug() << "Launched stats collector";
    delete config;
}


void SvdDataCollector::connectToDataStore() {
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


void SvdDataCollector::collectorGatherSlot() {
    if (not connected) {
        logDebug() << "Not connected. Reconnecting.";
        connectToDataStore();
    }

    /* perform data storage query: */
    redisReply *reply = (redisReply*)redisCommand(context, "HMSET stat rss 456 cpu 123");
    logTrace() << "HMSET REPLY:" << reply->str;
    freeReplyObject(reply);
}
