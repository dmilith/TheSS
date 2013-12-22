/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __PUBLIC_WATCHER_H__
#define __PUBLIC_WATCHER_H__


#include "../service_spawner/file_events_manager.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/service_watcher.h"
#include "../service_spawner/service.h"
#include "../service_spawner/data_collector.h"

#include <QObject>
#include <QFile>
#include <QEventLoop>


class SvdPublicWatcher: public QObject {
    Q_OBJECT

    private:
        SvdFileEventsManager *fileEvents = NULL;
        QSet<QString> fileEntries, domains;


    public:
        SvdPublicWatcher();
        ~SvdPublicWatcher();
        void reindexPublicDir();
        void invokeDirChangedTrigger();
        void invokeFileChangedTrigger(const QString& file);
        void processEntries(QSet<QString> newEntries);
        void validateDomainExistanceFor(QString file);
        void loadExistingDomains();


    signals:
        void autostartUser();
        void shutdownUser();


    public slots:
        void shutdownSlot();
        void dirChangedSlot(const QString& dir);
        void fileChangedSlot(const QString& file);

};


#endif
