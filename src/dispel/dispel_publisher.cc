/*
    Author: Daniel (dmilith) Dettlaff
    based on fragment of nzmqt implementation by Johann Duscher (a.k.a. Jonny Dee)

    © 2013 - VerKnowSys
*/


#include "dispel_publisher.h"


void Publisher::startImpl() {
    nodeUuid = currentNodeUUID();
    logInfo() << "Launching Publisher with id:" << nodeUuid << "on address:" << address();
    try {
        socket()->bindTo(address());
    } catch (std::exception& ex) {
        logError() << "Exception thrown in publisher: " << ex.what();
        logFatal() << "Publisher requires free tcp address:" << address() << " to work. Exitting!";
    }
    QTimer::singleShot(DISPEL_NODE_PUBLISHER_PAUSE, this, SLOT(sendIDLEJobMessage()));
}


void Publisher::sendIDLEJobMessage() {
    static quint64 counter = 0;
    auto noOp = "{}";
    QByteArray *msg = new QByteArray();
    msg->append(channelName());
    msg->append(noOp);
    assert(socket());
    logTrace() << "Publishing IDLE:" << msg->data() << "no:" << (counter++);
    socket()->setIdentity(channelName());
    socket()->sendMessage(msg->data());
    emit sentJobMessage(*msg);
    delete msg;

    QTimer::singleShot(DISPEL_NODE_PUBLISHER_PAUSE, this, SLOT(sendIDLEJobMessage()));
}
