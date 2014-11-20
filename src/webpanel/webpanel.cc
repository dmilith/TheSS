/**
 *  @author dmilith
 *
 *   © 2014 - VerKnowSys
 *
 */

#include "webpanel.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>


QT_USE_NAMESPACE


SvdWebPanel::SvdWebPanel(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"), QWebSocketServer::NonSecureMode, this)),
    m_clients() {

        if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
            logDebug() << "Echoserver listening on port" << port;
            connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &SvdWebPanel::onNewConnection);
            connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &SvdWebPanel::closed);
        }
    }

SvdWebPanel::~SvdWebPanel() {
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}


void SvdWebPanel::onNewConnection() {
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &SvdWebPanel::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &SvdWebPanel::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &SvdWebPanel::socketDisconnected);

    m_clients << pSocket;
}


void SvdWebPanel::processTextMessage(QString message) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendTextMessage(message);
    }
    m_pWebSocketServer->close();
}


void SvdWebPanel::processBinaryMessage(QByteArray message) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}


void SvdWebPanel::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
