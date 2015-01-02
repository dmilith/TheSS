/**
 *  @author dmilith
 *
 *   © 2014 - VerKnowSys
 *
 */

#include "api.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include "../yajl/api/yajl_tree.h"
#include "../core/json_api.h"
#include <QtCore/QDebug>


QT_USE_NAMESPACE


SvdAPI::SvdAPI(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("API Server"), QWebSocketServer::NonSecureMode, this)),
    m_clients() {

        if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
            logDebug() << "API server listening on port" << port;
            connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &SvdAPI::onNewConnection);
            connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &SvdAPI::closed);
        }
    }

SvdAPI::~SvdAPI() {
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}


void SvdAPI::onNewConnection() {
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &SvdAPI::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &SvdAPI::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &SvdAPI::socketDisconnected);

    m_clients << pSocket;
}


void SvdAPI::executeCommand(QString command) {
    /*
    format:
        {"cmd": "API command name", "serviceName": "service name"}
        {"cmd": "API command name", "serviceName": "service name", "hooks": ["stop", "reconfigure"]}
     */

    auto apiCommands = QStringList();
    apiCommands << "serviceList" << "serviceDetail" << "serviceHooks" <<
                    "serviceCreate" << "serviceDestroy" << "serviceSetAutostart" <<
                    "serviceEnableNotification" << "serviceEditIgniter";

    enum APIMAP {
        serviceList, serviceDetail, serviceHooks,
        serviceCreate, serviceDestroy, serviceSetAutostart,
        serviceEnableNotification, serviceEditIgniter
    };

    /* parsing JSON */
    char errbuf[1024];
    auto node = yajl_tree_parse(command.toUtf8(), errbuf, sizeof(errbuf));
    if (QString(errbuf).length() > 0) {
        logError() << "ERR:" << errbuf;
        return;
    }

    /* processing valid api params */
    QString cmd = JSONAPI::getString(node, NULL, "cmd");
    QString serviceName = JSONAPI::getString(node, NULL, "serviceName");
    if (cmd.isEmpty()) {
        logDebug() << "ERR: Empty API command!";
        return;
    }
    QStringList hooks = JSONAPI::getArray(node, NULL, "hooks");

    /* check if API command is on command list */
    if (not apiCommands.contains(cmd)) {
        logError() << "ERR: Command not recognized:" << cmd;
        return;
    }

    logDebug() << "cmd" << cmd;
    logDebug() << "serviceName" << serviceName;
    logDebug() << "hooks" << hooks;

    /* map commands */
    APIMAP comm;
    for (int i = 0; i < apiCommands.size(); i++) {
        if (cmd == apiCommands.at(i)) {
            comm = (APIMAP)i;
        }
    }

    /* execute command */
    switch (comm) {

        case serviceList: {
            sendListServices();
        }; break;

        case serviceDetail: {

        }; break;

        case serviceHooks: {

        }; break;

        case serviceCreate: {

        }; break;

        case serviceDestroy: {

        }; break;

        case serviceSetAutostart: {

        }; break;

        case serviceEnableNotification: {

        }; break;

        case serviceEditIgniter: {

        }; break;

        default: break;
    }

}


void SvdAPI::processTextMessage(QString command) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        logDebug() << "Processing API command:" << command << "for client from:" << pClient->localAddress().toString();
        executeCommand(command);
        // pClient->sendTextMessage(command);
    }
    // m_pWebSocketServer->close();
}


void SvdAPI::processBinaryMessage(QByteArray message) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}


void SvdAPI::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}


QString SvdAPI::getServiceStatus(QString name) {
    return "a name";
}


void SvdAPI::sendListServices() {
    logDebug() << "Sending service list";
    QString services;
    QStringList serviceList = QDir(getSoftwareDataDir()).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    QStringList finalList;
    Q_FOREACH(QString elem, serviceList) {
        finalList << "{\"name\": \"" + elem + "\", \"cpu\": 10, \"mem\": 2048, \"status\": \"" + getServiceStatus(elem) +
                     "\"}";
    }
    services += "[" + finalList.join(",") + "]";

    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"ts\": \"" +
                                QString::number(QDateTime::currentMSecsSinceEpoch()) + "\", \"method\": \"serviceList\", \"services\": " + services + "}");
    }
}


void SvdAPI::sendMessageToAllClients(QString name, QString reason, QString hookName) {
    logDebug() << "Sending status to all clients from service:" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"hook\": \"" + hookName + "\", \"reason\": \"" + reason + "\", \"ts\": \"" + QString::number(QDateTime::currentMSecsSinceEpoch()) + "\", \"method\": \"hookCallback\"}");
    }
}


void SvdAPI::installService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "installService");
}

void SvdAPI::configureService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "configureService");
}

void SvdAPI::reConfigureService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "reConfigureService");
}

void SvdAPI::reConfigureWithoutDepsService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "reConfigureWithoutDepsService");
}

void SvdAPI::validateService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "validateService");
}

void SvdAPI::startService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "startService");
}

void SvdAPI::startWithoutDepsService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "startWithoutDepsService");
}

void SvdAPI::afterStartService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "afterStartService");
}

void SvdAPI::stopService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "stopService");
}

void SvdAPI::stopWithoutDepsService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "stopWithoutDepsService");
}

void SvdAPI::afterStopService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "afterStopService");
}

void SvdAPI::restartService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "restartService");
}

void SvdAPI::restartWithoutDepsService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "restartWithoutDepsService");
}

void SvdAPI::reloadService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "reloadService");
}

void SvdAPI::destroyService(QString name, QString reason) {
    sendMessageToAllClients(name, reason, "destroyService");
}
