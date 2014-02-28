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

    QString message;

    /* make sure that every notification begins with proper data and time */
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    message = QString::number(now) + ": " + notificationMessage;

    QString notificationRoot = QString(getenv("HOME")) + SOFTWARE_DATA_DIR;
    if (getuid() == 0) {
        notificationRoot = QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR;
    }
    QString postfix;
    switch (level) {
        case NOTIFY:
            logInfo() << notificationMessage;
            postfix = ".notice";
            break;

        case WARNING:
            logWarn() << notificationMessage;
            postfix = ".warning";
            break;

        case ERROR:
            logError() << notificationMessage;
            postfix = ".error";
            break;

        case FATAL:
            logError() << notificationMessage;
            postfix = ".fatal";
            break;

    }

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

    if (level > WARNING) { /* notification only for errors */
        logInfo() << "Launching https error notification with message:" << message;

        QSslSocket socket;
        socket.connectToHostEncrypted("slack.com", 443);
        if (socket.waitForConnected())
            logDebug() << "SSL Connected!";

        logDebug() << "SSL Socket state:" << socket.state();
        socket.waitForEncrypted();

        QByteArray encodedMessage = message.toUtf8();
        encodedMessage = encodedMessage.toPercentEncoding();

        QString get = QString("GET") + " /api/chat.postMessage?token=" + NOTIFICATIONS_AUTH_TOKEN + "&channel=" + NOTIFICATIONS_CHANNEL_NAME + "&text=" + encodedMessage + "&username=" + NOTIFICATIONS_USERNAME + " HTTP/1.1\r\n";
        socket.write(get.toUtf8().data());
        socket.write("Host: slack.com\r\n");
        socket.write("Connection: Close\r\n\r\n");

        while (socket.waitForReadyRead())
            logDebug() << "Notification http response:" << socket.readAll().data();

        socket.close();
    }

    moveOldNotificationsToHistoryAndCleanHistory(notificationRoot, historyRoot);
}
