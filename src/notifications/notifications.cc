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


void notification(const QString& notificationMessage, const QString& serviceName, NotificationLevels level) {

    QString message;

    /* make sure that every notification begins with proper data and time */
    const QDateTime now = QDateTime::currentDateTime();
    if (not notificationMessage.startsWith(now.toString("dd-hh:mm"))) {
        message = now.toString("dd-hh:mm:ss - ") + notificationMessage;
    } else
        message = notificationMessage;

    QString notificationRoot = QString(getenv("HOME")) + SOFTWARE_DATA_DIR;
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
            logFatal() << notificationMessage;
            postfix = ".fatal";
            break;

    }

    if (history[message] > 0) {
        history[message]++;
    } else {

        if (serviceName.isEmpty()) {
            notificationRoot += NOTIFICATIONS_DATA_DIR;
            getOrCreateDir(notificationRoot); /* create it only for common notifications - unrelated to services*/

            auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
            QString content = message;
            hash->addData(content.toUtf8(), content.length());
            QString notificationFileName = hash->result().toHex() + postfix;
            delete hash;
            writeToFile(notificationRoot + "/" + notificationFileName, message);

        } else { /* it's service side notification! */
            notificationRoot += "/" + serviceName + NOTIFICATIONS_DATA_DIR;
            getOrCreateDir(notificationRoot); /* create it only for common notifications - unrelated to services*/

            auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
            QString content = message;
            hash->addData(content.toUtf8(), content.length());
            QString notificationFileName = hash->result().toHex() + postfix;
            delete hash;
            writeToFile(notificationRoot + "/" + notificationFileName, message);

        }
    }
}
