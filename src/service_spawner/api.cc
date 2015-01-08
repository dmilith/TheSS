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


QString escapeJsonString(const std::string& input) {
    QString ss;
    for (auto iter = input.begin(); iter != input.end(); iter++) {
        switch (*iter) {
            case '\\': ss += "\\\\"; break;
            case '"': ss += "\\\""; break;
            // case '/': ss += "\\/"; break;
            case '\b': ss += "\\b"; break;
            case '\f': ss += "\\f"; break;
            case '\n': ss += "\\n"; break;
            case '\r': ss += "\\r"; break;
            case '\t': ss += "\\t"; break;
            default: ss += *iter; break;
        }
    }
    return ss;
}


QString getJSONProcessesList(uint uid) {
    #ifdef __APPLE__
        /* NYI */
        return "[]";
    #endif
    #ifdef __FreeBSD__
        int count = 0;
        char** args = NULL;
        QString command, output;
        int pagesize = getpagesize();

        kvm_t* kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
        if (kd == 0) {
            logError() << "Error initializing kernel descriptor!";
            return "[]";
        }

        kinfo_proc* procs = kvm_getprocs(kd, KERN_PROC_UID, uid, &count); // get processes directly from BSD kernel
        if (count <= 0) {
            logError() << "No processes for given UID!";
            return "[]";
        }

        for (int i = 0; i < count; ++i) {
            QString out;
            command = "";
            args = kvm_getargv(kd, procs, 0);
            for (int y = 0; (args != 0) && (args[y] != 0); y++)
                if (y == 0)
                    command = QString(args[y]);
                else
                    command += " " + QString(args[y]);

            unsigned int cnt = 0;
            int error;
            char errbuf[_POSIX2_LINE_MAX];
            struct vnstat vn;
            struct procstat* procstat = procstat_open_sysctl();
            struct kinfo_proc *kproc = procstat_getprocs(procstat, KERN_PROC_PID, procs->ki_pid, &cnt);

            QString fileStat = "[";
            if (cnt != 0) {
                struct filestat_list *filesInfo = nullptr;
                struct filestat *fst;

                filesInfo = procstat_getfiles(procstat, kproc, 0);
                if (filesInfo != nullptr) {
                    STAILQ_FOREACH(fst, filesInfo, next) {
                        fileStat += "{";
                        error = procstat_get_vnode_info(procstat, fst, &vn, errbuf);
                        if (error != 0) {
                            fileStat += "}";
                            continue;
                        }
                        fileStat += QString("vn_size:") += QString::number(vn.vn_size) += QString(",vn_mntdir:\"") += QString(vn.vn_mntdir) += QString("\",fileid:") += QString::number(vn.vn_fileid) += QString(", vn_dev:\"") += QString::number(vn.vn_dev) += QString("\",vn_fsid:") += QString::number(vn.vn_fsid) += QString(",vn_type:") += QString::number(vn.vn_type) += QString(",vn_mode:") += QString::number(vn.vn_mode) += QString(",vn_devname:\"") += QString(vn.vn_devname) += QString("\"");
                        fileStat += "}";
                    }
                }
            }
            fileStat += "]";
            fileStat = fileStat.replace("}{","},{");

            procstat_freeprocs(procstat, kproc);
            procstat_close(procstat);
            procstat = nullptr;
            kproc = nullptr;

            if (i == 0) {
                out += "[";
            }
            out += "{\"pid\":" + QString::number(procs->ki_pid) + ","
                + "\"ppid\":" + QString::number(procs->ki_ppid) + ","
                + "\"name\":\"" + escapeJsonString(procs->ki_comm) + "\","
                + "\"cmd\":\"" + escapeJsonString(command.toStdString()) + "\","
                + "\"rss\":" + QString::number(procs->ki_rssize * pagesize) + ","
                + "\"mrss\":" + QString::number(procs->ki_rusage.ru_maxrss * pagesize) + ","
                + "\"runtime\":" + QString::number(procs->ki_runtime / 1000) + ","
                + "\"blk-in\":" + QString::number(procs->ki_rusage.ru_inblock) + ","
                + "\"blk-out\":" + QString::number(procs->ki_rusage.ru_oublock) + ","
                + "\"thr\":" + QString::number(procs->ki_numthreads) + ","
                + "\"priority\":" + QString::number(procs->ki_pri.pri_level) + ","
                + "\"files-stat\":" + fileStat + "}";

            if (i == count - 1) {
                out += "]";
            } else {
                out += ",";
            }
            args = nullptr;
            output += out;
            procs++;
        }
        kvm_close(kd);
        return output;
    #endif
}


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
    QPointer<QWebSocket> pSocket = m_pWebSocketServer->nextPendingConnection();

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
                    "serviceEnableNotification" << "serviceEditIgniter" << "processStat";

    enum APIMAP {
        serviceList, serviceDetail, serviceHooks,
        serviceCreate, serviceDestroy, serviceSetAutostart,
        serviceEnableNotification, serviceEditIgniter, processStat
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
    // QString serviceName = JSONAPI::getString(node, NULL, "serviceName");
    // if (cmd.isEmpty()) {
    //     logDebug() << "ERR: Empty API command!";
    //     return;
    // }
    QStringList hooks = JSONAPI::getArray(node, NULL, "hooks");

    /* check if API command is on command list */
    if (not apiCommands.contains(cmd)) {
        logError() << "ERR: Command not recognized:" << cmd;
        return;
    }

    logDebug() << "cmd" << cmd;
    // logDebug() << "serviceName" << serviceName;
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

        case processStat: {
            sendUserStatsToAllClients();
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
        finalList << "{\"name\": \"" + elem + "\"}";
    }
    services += "[" + finalList.join(",") + "]";

    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"ts\": \"" +
                                QString::number(QDateTime::currentMSecsSinceEpoch()) + "\", \"method\": \"serviceList\", \"result\": " + services + "}");
    }
}


void SvdAPI::sendUserStatsToAllClients() {
    logDebug() << "Sending process stats to all clients";
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage(getJSONProcessesList(getuid())); /* NOTE: takes only processes of current UID */
    }
}


void SvdAPI::sendCustomMessageToAllClients(QString name, QString content) {
    logDebug() << "Sending custom message to all clients";
    Q_FOREACH(auto client, m_clients) {
        logDebug() << "Connected peer:" << client->peerAddress();
        client->sendTextMessage("{\"serviceName\": \"" + name + "\", " + content + "}");
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
