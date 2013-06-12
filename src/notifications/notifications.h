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


enum NotificationLevels {NOTIFY, WARNING, ERROR, FATAL};


static QMap<QString, int> history = QMap<QString, int>(); /* content, amount */


void notification(const QString& notificationMessage, const QString& serviceName = "", NotificationLevels level = ERROR);


#endif
