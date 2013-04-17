/**
 *  @author tallica
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __WEBAPP_WATCHER_H__
#define __WEBAPP_WATCHER_H__

#include "file_events_manager.h"
#include "service_config.h"
#include "service_watcher.h"
#include "webapp_deployer.h"
#include "service.h"

#include <QObject>
#include <QFile>
#include <QEventLoop>


class SvdWebAppHookTriggerFiles {

    public:
        SvdWebAppHookTriggerFiles(const QString& path);
        ~SvdWebAppHookTriggerFiles();

        SvdHookTriggerFile *start, *stop, *restart, *reload;

};


class SvdWebAppHookIndicatorFiles {

    public:
        SvdWebAppHookIndicatorFiles(const QString& path);
        ~SvdWebAppHookIndicatorFiles();

        SvdHookIndicatorFile *autostart, *running;

};


class SvdWebAppWatcher: public QObject {
    Q_OBJECT

    public:
        SvdWebAppWatcher(const QString& name);
        ~SvdWebAppWatcher();

    private:
        SvdFileEventsManager *fileEvents;
        SvdWebAppHookTriggerFiles *triggerFiles;
        SvdWebAppHookIndicatorFiles *indicatorFiles;
        SvdWebAppDeployer *webApp;
        QString webAppDir;

        void cleanupTriggerHookFiles();

    signals:
        void startWebApp();
        void stopWebApp();
        void restartWebApp();
        void reloadWebApp();

    public slots:
        void shutdownSlot();
        void dirChangedSlot(const QString& dir);
        void fileChangedSlot(const QString& file);

};


#endif