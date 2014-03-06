/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef __NOTIFICATIONS_CENTER__
#define __NOTIFICATIONS_CENTER__


#include <QtCore>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslSocket>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../service_spawner/utils.h"
#include <sys/ioctl.h>


enum NotificationLevels {NOTIFY = 0, WARNING, ERROR, FATAL};


struct Notification {
    int level;
    QString content;
    qint64 time;
};


void notification(const QString& notificationMessage, NotificationLevels level = NOTIFY);


#endif
