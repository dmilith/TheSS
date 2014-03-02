/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "../globals/globals.h"
#include "../service_spawner/process.h"
#include "../death_watch/death_watch.h"
#include "notifications.h"


void moveOldNotificationsToHistoryAndCleanHistory(const QString& notificationRoot, const QString& historyRoot) {
    /* count notifications on "current" list */
    auto items = QDir(notificationRoot).entryList(QDir::Files, QDir::Time);
    int notificationAmount = items.size();
    logDebug() << "Notification items in dir:" << notificationRoot << ":" << QString::number(notificationAmount);

    /* moving old items to history */
    for (int i = notificationAmount - 1; i > 0; i--) {
        if (i > NOTIFICATIONS_LAST_SHOWN) {
            QString item = items.at(i);
            logDebug() << "Moving old notification to history:" << item;
            QFile::copy(notificationRoot + "/" + item, historyRoot + "/" + item);
            QFile::remove(notificationRoot + "/" + item);
        }
    }

    /* history max amount check */
    items = QDir(historyRoot).entryList(QDir::Files, QDir::Time);
    notificationAmount = items.size();
    logDebug() << "Notification history items in dir:" << historyRoot << ":" << QString::number(notificationAmount);
    for (int i = notificationAmount - 1; i > 0; i--) {
        if (i > NOTIFICATIONS_HISTORY_KEEP_UPTO) {
            QString item = items.at(i);
            logDebug() << "Removing old notification from notification history:" << item;
            QFile::remove(historyRoot + "/" + item);
        }
    }
}


void notification(const QString& notificationMessage, NotificationLevels level) {

    QString message, levelStr, icon, postfix;

    /* make sure that every notification begins with proper data and time */
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QDateTime timeNow = QDateTime::currentDateTime();

    QString notificationRoot = QString(getenv("HOME")) + SOFTWARE_DATA_DIR;
    if (getuid() == 0) {
        notificationRoot = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR;
    }
    switch (level) {
        case NOTIFY:
            logInfo() << notificationMessage;
            postfix = ".notice";
            levelStr = "NOTIFY";
            icon = ":new:";
            break;

        case WARNING:
            logWarn() << notificationMessage;
            postfix = ".warning";
            levelStr = "WARNING";
            icon = ":cold_sweat:";
            break;

        case ERROR:
            logError() << notificationMessage;
            postfix = ".error";
            levelStr = "ERROR";
            icon = ":rage:";
            break;

        case FATAL:
            logError() << notificationMessage;
            postfix = ".fatal";
            levelStr = "FATAL";
            icon = ":skull:";
            break;

    }
    message = timeNow.toString("hh:mm:ss.zzz") + "     " + levelStr + " @ " + QHostInfo::localHostName() +
                            "\n\t\t\t\t\t\t\t\t    " + notificationMessage;

    QString historyRoot = notificationRoot + NOTIFICATIONS_HISTORY_DATA_DIR;
    notificationRoot += NOTIFICATIONS_DATA_DIR;
    getOrCreateDir(notificationRoot); /* create it only for common notifications - unrelated to services*/
    getOrCreateDir(historyRoot);

    auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
    QString content = message;
    hash->addData(content.toUtf8(), content.length());
    QString notificationFileName = hash->result().toHex() + postfix;
    delete hash;
    logDebug() << "Notification msg: " << message << " written to:" << QString::number(now) + "_" + notificationFileName;
    writeToFile(notificationRoot + "/" + QString::number(now) + "_" + notificationFileName, message);

    // if (level > NOTIFY) { /* notification only for errors */
        logInfo() << "Launching https error notification with message:" << message;

        QSslSocket socket;
        #ifdef __FreeBSD__
            if (not QFile::exists(DEFAULT_SSL_CA_CERT)) {
                logWarn() << "No default SSL ROOT CA Certificates found in:" << DEFAULT_SSL_CA_CERT;
            } else {
                QSslSocket::addDefaultCaCertificates(DEFAULT_SSL_CA_CERT);
                socket.addCaCertificates(DEFAULT_SSL_CA_CERT);
                logTrace() << "CA certificates:" << socket.caCertificates();
            }
        #endif
        socket.connectToHostEncrypted(DEFAULT_API_HOST, DEFAULT_SSL_PORT);
        if (socket.waitForEncrypted())
            logDebug() << "Connected to SSL host:" << DEFAULT_API_HOST;

        logDebug() << "SSL Socket state:" << socket.state();

        QByteArray encodedMessage = message.toUtf8();
        encodedMessage = encodedMessage.toPercentEncoding();

        QString get = QString("GET") + " /api/chat.postMessage?token=" + NOTIFICATIONS_AUTH_TOKEN + "&channel=" + NOTIFICATIONS_CHANNEL_NAME + "&text=" + encodedMessage + "&username=" + NOTIFICATIONS_USERNAME + "&icon_emoji=" + icon + "&parse=full" + " HTTP/1.1\r\n";
        logDebug() << "SSL request:" << get;
        socket.write(get.toUtf8().data());
        get = QString("Host: " + QString(DEFAULT_API_HOST) + "\r\n");
        socket.write(get.toUtf8().data());
        socket.write("Connection: Close\r\n\r\n");

        while (socket.waitForBytesWritten())
            logInfo() << "Notification http response:" << socket.readAll().data();

        socket.close();
    // }

    moveOldNotificationsToHistoryAndCleanHistory(notificationRoot, historyRoot);
}
