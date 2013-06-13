/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include <QtCore>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../service_spawner/utils.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    QStringList args = app.arguments();

    /* Logger setup */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setDetailsLevel(Logger::Debug);

    logInfo() << "Notifications manager v0.1.0";

    QStringList available;
    QString userSoftwarePrefix = QString(getenv("HOME")) + QString(SOFTWARE_DATA_DIR);
    auto userSoftware = QDir(userSoftwarePrefix).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    /* iterate through user software to find software notifications */
    Q_FOREACH(QString service, userSoftware) {

        logDebug() << "Found service:" << service;
        QString notificationsPrefix = userSoftwarePrefix + "/" + service + NOTIFICATIONS_DATA_DIR;
        if (QDir().exists(notificationsPrefix)) {
            QStringList notificationsByDateSource = QDir(notificationsPrefix).entryList(QDir::Files, QDir::Time);
            QStringList notificationsByDate;
            notificationsByDate.reserve(notificationsByDateSource.size());
            reverse_copy(notificationsByDateSource.begin(), notificationsByDateSource.end(), back_inserter(notificationsByDate)); /* C++ list reverse.. huh */

            Q_FOREACH(QString notify, notificationsByDate) {
                QString notificationFile = notificationsPrefix + "/" + notify;
                QString content = QString(readFileContents(notificationFile).c_str()).trimmed();
                logDebug() << "Notification:" << notify;
                logInfo() << "Notification:" << content;

                // logDebug() << "Moving notification to history: " << notificationFile;

                // XXX currently just remove notices after read
                // if (notificationFile.endsWith(".notice"))
                    // QFile::remove(notificationFile);
            }
        }
    }

    // auto userEntries = QDir(QString(getenv("HOME")) + QString(SOFTWARE_DATA_DIR)).entryList(QDir::Dirs & QDir::NoDotAndDotDot);
    // auto thessEntries = QDir(QString(DEFAULTSOFTWARETEMPLATESDIR)).entryList(QDir::Files);
    // auto rootEntries = QStringList();
    // if (getuid() == 0) {
    //         rootEntries << QDir(QString(SYSTEM_USERS_DIR) + QString(DEFAULTUSERIGNITERSDIR)).entryList(QDir::Files);
    // }

    // available.clear();
    // available.append(userEntries);
    // // available.append(standardEntries);
    // available.append(rootEntries);
    // available.removeDuplicates();
    // available.sort();
    // available.replaceInStrings(QRegExp("\\.json"), "");



    return app.exec();
}
