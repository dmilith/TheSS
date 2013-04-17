/**
 *  @author tallica
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __FILE_EVENTS_MANAGER_H__
#define __FILE_EVENTS_MANAGER_H__

#include <QFileSystemWatcher>


class SvdFileEventsManager: public QFileSystemWatcher {

    public:
        void registerFile(const QString& path);
        void unregisterFile(const QString& path);
        bool isWatchingFile(const QString& path);
        bool isWatchingDir(const QString& path);

};


#endif