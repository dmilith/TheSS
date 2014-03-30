/*
    Author: Daniel (dmilith) Dettlaff
    based on fragment of nzmqt implementation by Johann Duscher (a.k.a. Jonny Dee)

    © 2013 - VerKnowSys
*/


#ifndef __DISPEL_PUBLISHER__
#define __DISPEL_PUBLISHER__


#include <QCoreApplication>
#include "dispel_core.h"


class Publisher: public AbstractZmqBase {
    Q_OBJECT
    typedef AbstractZmqBase super;


    private:
        QString nodeUuid;
        QString address_;
        QString channel_;
        ZMQSocket* socket_;


    protected:
        void startImpl();


    signals:
        void sentJobMessage(const QByteArray message);


    protected slots:
        void sendIDLEJobMessage();


    public:
        explicit Publisher(ZMQContext& context, const QString& address, const QString& channel, QObject* parent = 0): super(parent), address_(address), channel_(channel), socket_(0) {
                assert(context);
                assert(!address_.isEmpty());
                assert(!address.isEmpty());
                assert(!channel_.isEmpty());
                assert(!channel.isEmpty());
                socket_ = context.createSocket(ZMQSocket::TYP_XPUB, this); // TYP_PUB
                assert(socket_);
                socket_->setObjectName("Publisher.Socket.socket(PUB) -> " + channel);
                logDebug() << "Publisher created for channel:" << channel << "address:" << address << "parent:" << parent;
                this->thread()->moveToThread(QCoreApplication::instance()->thread());
                setParent(QCoreApplication::instance());
        }


        inline ZMQSocket* socket() {
            return socket_;
        }


        inline QString id() {
            return nodeUuid;
        }


        inline QString channelName() {
            return channel_;
        }


        inline QString address() {
            return address_;
        }

};


#endif
