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

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../service_spawner/utils.h"
#include <sys/ioctl.h>


#define NOTIFICATION_LEVEL_ERROR    0
#define NOTIFICATION_LEVEL_WARNING  1
#define NOTIFICATION_LEVEL_NOTICE   2

struct Notification {
    int level;
    QString content;
    QDateTime time;
};

enum NotificationLevels {NOTIFY, WARNING, ERROR, FATAL};


static QMap<QString, int> history = QMap<QString, int>(); /* content, amount */


void notification(const QString& notificationMessage, const QString& serviceName = "", NotificationLevels level = ERROR);


#endif
