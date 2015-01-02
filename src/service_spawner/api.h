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


class SvdAPI: public QObject {

    Q_OBJECT

    public:
        explicit SvdAPI(quint16 port, QObject *parent = Q_NULLPTR);
        ~SvdAPI();

    Q_SIGNALS:
        void closed();

    private Q_SLOTS:
        void onNewConnection();
        void processTextMessage(QString message);
        void processBinaryMessage(QByteArray message);
        void socketDisconnected();

    public:
        QString getServiceStatus(QString name);
        void executeCommand(QString command);
        void sendListServices();
        void sendCustomMessageToAllClients(QString name, QString content);
        void sendMessageToAllClients(QString name, QString reason, QString hookName);

        void startService(QString name, QString reason = "");
        void installService(QString name, QString reason = "");
        void configureService(QString name, QString reason = "");
        void reConfigureService(QString name, QString reason = "");
        void reConfigureWithoutDepsService(QString name, QString reason = "");
        void validateService(QString name, QString reason = "");
        void startWithoutDepsService(QString name, QString reason = "");
        void afterStartService(QString name, QString reason = "");
        void stopService(QString name, QString reason = "");
        void stopWithoutDepsService(QString name, QString reason = "");
        void afterStopService(QString name, QString reason = "");
        void restartService(QString name, QString reason = "");
        void restartWithoutDepsService(QString name, QString reason = "");
        void reloadService(QString name, QString reason = "");
        void destroyService(QString name, QString reason = "");


    private:
        QWebSocketServer* m_pWebSocketServer;
        QList<QWebSocket*> m_clients;

};


#endif
