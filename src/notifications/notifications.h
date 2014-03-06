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


#define NOTIFICATION_LEVEL_FATAL    3
#define NOTIFICATION_LEVEL_ERROR    2
#define NOTIFICATION_LEVEL_WARNING  1
#define NOTIFICATION_LEVEL_NOTICE   0

struct Notification {
    int level;
    QString content;
    qint64 time;
};

enum NotificationLevels {NOTIFY = 0, WARNING, ERROR, FATAL};


void notification(const QString& notificationMessage, NotificationLevels level = NOTIFY);


#endif
