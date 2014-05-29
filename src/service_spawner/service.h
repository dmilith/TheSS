/**
 *  @author tallica
 *
 *   © 2013-2014 - VerKnowSys
 *
 */

#ifndef __SERVICE_H__
#define __SERVICE_H__


#include <sys/stat.h>
#include <sys/user.h>
#include <sys/sysctl.h>

#ifdef __APPLE__
    #include <sys/time.h>
    #include <mach/clock.h>
    #include <mach/clock_priv.h>
    #include <mach/clock_types.h>
    #include <sys/proc.h>
    #include <util.h>
    #include <sys/ioctl.h>
#endif

#ifdef __FreeBSD__
    #include <kvm.h>
    #include <sys/capability.h>
    #include <libutil.h>
#endif

#ifdef __linux__
    #include <sys/time.h>
    #include <sys/procfs.h>
    #include <sys/fcntl.h>
#endif

#include "../globals/globals.h"
#include "service_config.h"
#include "../core/utils.h"
#include "../notifications/notifications.h"
#include "../death_watch/death_watch.h"

#include <signal.h>
#include <QObject>
#include <QElapsedTimer>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>


class SvdService: public QThread {
    Q_OBJECT

    public:
        SvdService(const QString& name);
        const QString createHash(const QString& nme);
        const QString releasePostfix();
        qint64 getUptime();
        bool checkProcessStatus(pid_t pid);
        void notificationSend(const QString& notificationMessage);
        void notificationSend(const QString& notificationMessage, NotificationLevels level);
        void stopSitters();
        void loadServiceConfig(const QString& nme);

        static void setupDefaultVPNNetwork();
        static void shutdownDefaultVPNNetwork();

    protected:
        void run();

    private:
        // QMutex mtx;
        QString name;
        QString hash = "";
        QElapsedTimer uptime;
        QTimer babySitter, cronSitter;
        QList<SvdService*> dependencyServices;
        QNetworkAccessManager* networkManager = NULL;
        SvdServiceConfig* config = NULL;
        SvdServiceConfig* configCache = NULL;

    public slots:
        void cronSitterSlot();
        void babySitterSlot();
        void installSlot();
        void configureSlot();
        void reConfigureSlot();
        void reConfigureSlot(bool withDeps);
        void reConfigureWithoutDepsSlot();
        void startSlot();
        void startSlot(bool withDeps);
        void startWithoutDepsSlot();
        void afterStartSlot();
        void stopSlot();
        void stopSlot(bool withDeps);
        void stopWithoutDepsSlot();
        void afterStopSlot();
        void restartSlot();
        void restartSlot(bool withDeps);
        void restartWithoutDepsSlot();
        void reloadSlot();
        void validateSlot();
        void destroySlot();
        void finishedSlot(QNetworkReply* reply); /* network check slot */
};


#endif
