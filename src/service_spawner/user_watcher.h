/**
 *  @author tallica, dmilith
 *
 *   © 2013-2014 - VerKnowSys
 *
 */

#ifndef __USER_WATCHER_H__
#define __USER_WATCHER_H__

#include "file_events_manager.h"
#include "api.h"
#include "service_config.h"
#include "service_watcher.h"
#include "service.h"
#include "data_collector.h"

#include <QObject>
#include <QFile>
#include <QEventLoop>


class SvdUserHookTriggerFiles {

    public:
        SvdUserHookTriggerFiles(const QString& path);
        ~SvdUserHookTriggerFiles();

        SvdHookTriggerFile *shutdown, *graceShutdown;

};


class SvdUserHookIndicatorFiles {

    public:
        SvdUserHookIndicatorFiles(const QString& path);
        ~SvdUserHookIndicatorFiles();

        SvdHookIndicatorFile *autostart = NULL;

};


class SvdUserWatcher: public QObject {
    Q_OBJECT

    public:
        SvdUserWatcher();
        ~SvdUserWatcher();
        QList<SvdServiceWatcher *> serviceWatchers;
        QStringList services;
        QStringList webApps;

    protected:

    private:
        SvdDataCollector *dataCollector = NULL;
        SvdFileEventsManager *fileEvents = NULL;
        SvdUserHookTriggerFiles *triggerFiles = NULL;
        SvdUserHookIndicatorFiles *indicatorFiles = NULL;
        SvdAPI *apiServer = nullptr;
        QString homeDir;
        QString softwareDataDir;

        QMutex collectorMutex;

        void init();
        void collectServices();
        // void collectWebApplications();
        void checkUserControlTriggers();

    signals:
        void autostartUser();
        void shutdownUser();

    public slots:
        void shutdownSlot();
        void dirChangedSlot(const QString& dir);
        void fileChangedSlot(const QString& file);

};


#endif
