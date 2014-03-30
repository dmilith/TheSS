/*
    Author: Daniel (dmilith) Dettlaff
    based on fragment of nzmqt implementation by Johann Duscher (a.k.a. Jonny Dee)

    © 2013 - VerKnowSys
*/


#include "dispel_subscriber.h"


void Subscriber::messageReceived(const QList<QByteArray>& message) {
    // logDebug() << "Subscriber> " << message;
    emit pingReceived(message);
}


void Subscriber::startImpl() {
    nodeUuid = currentNodeUUID();
    logInfo() << "Launching Subscriber with id:" << nodeUuid << "trying address:" << address_ << "on channel" << channel_;
    assert(!nodeUuid.isEmpty());
    socket_->connectTo(address_);
    socket_->subscribeTo(channel_);
    logDebug() << "Subscribing to:" << address_ << "@channel:" << channel_;
}

