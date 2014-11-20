/**
 *  @author dmilith
 *
 *   © 2014 - VerKnowSys
 *
 */

#ifndef __WEBPANEL_H__
#define __WEBPANEL_H__

// #include "../core/utils.h"
#include <QtCore>
#include <QProcess>
#include <QtWebSockets/QtWebSockets>
#include <iostream>
#include <unistd.h>

#include "../globals/globals.h"
#include "../core/utils-core.h"
#include "../core/logger-core.h"


QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)


class SvdWebPanel: public QObject {

    Q_OBJECT

    public:
        explicit SvdWebPanel(quint16 port, QObject *parent = Q_NULLPTR);
        ~SvdWebPanel();

    Q_SIGNALS:
        void closed();

    private Q_SLOTS:
        void onNewConnection();
        void processTextMessage(QString message);
        void processBinaryMessage(QByteArray message);
        void socketDisconnected();

    private:
        QWebSocketServer* m_pWebSocketServer;
        QList<QWebSocket*> m_clients;

};


#endif
