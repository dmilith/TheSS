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


enum NotificationType {HIPCHAT, DEFAULT};
enum NotificationLevels {NOTIFY, WARNING, ERROR};


static QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager();
static QMap<QString, int> history = QMap<QString, int>(); /* content, amount */


class Notification {
    NotificationLevels level = WARNING;

    public:
        QString fromName() { return "TheSS"; };
        QString roomName() { return "DevOps"; };
        QString notifyColor() { return "green"; };
        QString warningColor() { return "yellow"; };
        QString errorColor() { return "red"; };
        QString notifyAll() { return "0"; };

        QString urlAddress();
        QString apiKey();

        void setLevel(NotificationLevels newLevel) { this->level = newLevel; };
        NotificationLevels getLevel() { return this->level; };
};


void notification(const QString& notificationMessage);
void notification(const QString& notificationMessage, NotificationType type);


#endif
