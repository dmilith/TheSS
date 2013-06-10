/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __NOTIFICATIONS_CENTER_HIPCHAT_TRANSPORT__
#define __NOTIFICATIONS_CENTER_HIPCHAT_TRANSPORT__


#include "../notifications.h"


class HipChatNotification: public Notification {

    public:

        // ~HipChatNotification() { if (networkAccessManager) delete networkAccessManager; };

        QString urlAddress() { return "https://api.hipchat.com/v1/rooms/message"; }


        QString apiKey() {
            QFile file(QString(getenv("HOME")) + "/.hipchat_api_key");
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return "NO_API_KEY_SPECIFIED";
            auto result = QString(file.readLine()).trimmed();
            file.close();
            return result;
        }


        void notification(const QString& notificationMessage) {
            // networkAccessManager = new QNetworkAccessManager();
            QString apiHttpUrl = this->urlAddress();
            QUrl url = QUrl(apiHttpUrl);
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

            QUrl params;
            qDebug() << this->urlAddress() << this->apiKey() << this->roomName() << this->fromName() << this->notifyAll() << this->notifyColor() << notificationMessage;

            params.addQueryItem("auth_token", this->apiKey());
            params.addQueryItem("room_id", this->roomName());
            params.addQueryItem("from", this->fromName());
            params.addQueryItem("notify", this->notifyAll()); /* don't notify all about it */
            params.addQueryItem("color", this->notifyColor()); /* don't notify all about it */
            params.addQueryItem("message", notificationMessage);

            QByteArray data;
            data = params.encodedQuery();
            QNetworkReply *reply = networkAccessManager->post(request, data);

            switch (reply->error()) {
                case QNetworkReply::NoError:
                    break;

                default:
                    qWarning() << "**********************************************************";
                    qWarning() << "* Notification error. Please check notification backend! *";
                    qWarning() << "**********************************************************";
                    break;
            }
        }

};

#endif
