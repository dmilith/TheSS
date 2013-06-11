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
    if (history[notificationMessage] > 0) {
        history[notificationMessage]++;
    } else {
        auto notification = new HipChatNotification();
        if (notificationMessage.contains("Error")) {
            notification->setLevel(ERROR);
        }
        notification->notification(notificationMessage);
        delete notification;
        history[notificationMessage]++;
    }
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
