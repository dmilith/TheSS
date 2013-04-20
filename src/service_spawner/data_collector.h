/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef __DATA_COLLECTOR_H__
#define __DATA_COLLECTOR_H__


#include <QThread>
#include <QTimer>

#include "../globals/globals.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/service.h"
#include "../hiredis/hiredis.h"


class SvdDataCollector: public QObject {
    Q_OBJECT

    public:
        SvdDataCollector();
        void connectToDataStore();


    public slots:
        void collectorGatherSlot();


    private:
        SvdService *service = NULL;
        redisContext *context = NULL;
        QTimer *collector = NULL;
        bool connected = false;
        const QString name = "ProcessDataCollector";
        QString socketFile = "";
        QString triggerFile = "";

};


#endif
