/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "../globals/globals.h"
#include "notifications.h"
#include "transports/hipchat.h"


void notification(const QString& notificationMessage) {
    auto notification = new HipChatNotification();
    notification->notification(notificationMessage);
    delete notification;
}


void notification(const QString& notificationMessage, NotificationType type) {
    switch (type) {
        case HIPCHAT: {
            auto notification = new HipChatNotification();
            notification->notification(notificationMessage);
            delete notification;

        } break;

        case DEFAULT: {
            qDebug() << "default badziąg! NYI.";

        } break;

        default: {
            qDebug() << "default badziąg! NYI.";
        } break;
    }
}
