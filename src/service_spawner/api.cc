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


void executeCommand(QString command) {
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


void SvdAPI::installService(QString name, QString reason) {
    logWarn() << "installService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"hook\": \"installService\", \"reason\": \"" + reason + "\"}");
    }
}

void SvdAPI::configureService(QString name, QString reason) {
    logWarn() << "configureService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"configureService\"}");
    }
}

void SvdAPI::reConfigureService(QString name, QString reason) {
    logWarn() << "reConfigureService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"reConfigureService\"}");
    }
}

void SvdAPI::reConfigureWithoutDepsService(QString name, QString reason) {
    logWarn() << "reConfigureWithoutDepsService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"reConfigureWithoutDepsService\"}");
    }
}

void SvdAPI::validateService(QString name, QString reason) {
    logWarn() << "validateService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"validateService\"}");
    }
}

void SvdAPI::startService(QString name, QString reason) {
    logWarn() << "startService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"startService\"}");
    }
}

void SvdAPI::startWithoutDepsService(QString name, QString reason) {
    logWarn() << "startWithoutDepsService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"startWithoutDepsService\"}");
    }
}

void SvdAPI::afterStartService(QString name, QString reason) {
    logWarn() << "afterStartService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"afterStartService\"}");
    }
}

void SvdAPI::stopService(QString name, QString reason) {
    logWarn() << "stopService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"stopService\"}");
    }
}

void SvdAPI::stopWithoutDepsService(QString name, QString reason) {
    logWarn() << "stopWithoutDepsService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"stopWithoutDepsService\"}");
    }
}

void SvdAPI::afterStopService(QString name, QString reason) {
    logWarn() << "afterStopService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"afterStopService\"}");
    }
}

void SvdAPI::restartService(QString name, QString reason) {
    logWarn() << "restartService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"restartService\"}");
    }
}

void SvdAPI::restartWithoutDepsService(QString name, QString reason) {
    logWarn() << "restartWithoutDepsService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"restartWithoutDepsService\"}");
    }
}

void SvdAPI::reloadService(QString name, QString reason) {
    logWarn() << "reloadService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"reloadService\"}");
    }
}

void SvdAPI::destroyService(QString name, QString reason) {
    logWarn() << "destroyService" << name;
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", \"reason\": \"" + reason + "\", \"hook\": \"destroyService\"}");
    }
}
