/**
 *  @author tallica
 *
 *   © 2013 - VerKnowSys
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
#include "utils.h"
#include "../death_watch/death_watch.h"

#include <signal.h>
#include <QObject>
#include <QElapsedTimer>


class SvdService: public QThread {
    Q_OBJECT

    public:
        SvdService(const QString& name);
        ~SvdService();
        qint64 getUptime();
        bool checkProcessStatus(pid_t pid);

    protected:
        void run();

    private:
        QString name;
        QElapsedTimer *uptime = NULL;
        QTimer *babySitter = NULL;
        SvdProcess *serverProcess = NULL;
        QList<SvdService*> dependencyServices;


    public slots:
        void babySitterSlot();
        void installSlot();
        void configureSlot();
        void startSlot();
        void afterStartSlot();
        void stopSlot();
        void afterStopSlot();
        void restartSlot();
        void reloadSlot();
        void validateSlot();
};


#endif