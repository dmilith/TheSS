/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef __WEBAPP_DEPLOYER__
#define __WEBAPP_DEPLOYER__


#include "../jsoncpp/json/json.h"
#include "service_config.h"
#include "webapp_types.h"
#include "utils.h"
#include "service.h"
#include "process.h"
#include <QObject>
#include <QtTest/QtTest>



class SvdWebAppDeployer: public QObject {
    Q_OBJECT

    WebAppTypes appType = NoType;
    QString typeName, domain;

    public:
        SvdWebAppDeployer(const QString& domain);
        ~SvdWebAppDeployer();
        WebAppTypes getType();
        QString getTypeName();
        QString getDomain();
        void detect(); /* used to detect application type */


    public slots:
        void startSlot();
        void stopSlot();
        void restartSlot();
        void reloadSlot();
};


#endif
