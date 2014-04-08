/**
 *  @author dmilith
 *
 *   Software config loader for json igniters.
 *   © 2013-2014 - VerKnowSys
 *
 */

#ifndef __SERVICE_CONFIG__
#define __SERVICE_CONFIG__

#include "../globals/globals.h"
#include "../yajl/api/yajl_tree.h"
#include "../notifications/notifications.h"
#include "../core/utils.h"
#include "cron_entry.h"
#include "../core/json_api.h"

#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QNetworkInterface>
#include <iostream>


class SvdScheduler {

    public:
        SvdScheduler(const QString& initialCronEntry, const QString& initialCommands);
        QString cronEntry, commands, sha;

};


class SvdShellOperations {
    public:
        SvdShellOperations();
        SvdShellOperations(const QString& initialCommand, const QString& initialExpectOutput);

        QString commands;
        QString expectOutput;

};


class SvdServiceConfig : public QObject {
    Q_OBJECT

    public:
        SvdServiceConfig(); /* Load default values */
        ~SvdServiceConfig();
        SvdServiceConfig(const QString& serviceName, bool dryRun = false);

        const QString replaceAllSpecialsIn(const QString content);
        const QString userServiceRoot();
        const QString serviceRoot();
        const QString prefixDir();
        const QString defaultTemplateFile();
        const QString rootIgniter();
        const QString userIgniter();
        const QString releaseName();

        /* internal service config API. Uses shared JSON loader */
        QString getString(const char* element);
        QStringList getArray(const char* element);
        bool getBoolean(const char* element);
        long long getInteger(const char* element);
        double getDouble(const char* element);
        /* end of service JSON API */

        bool serviceInstalled();
        bool serviceConfigured();

        QString loadDefaultIgniter();
        QString loadIgniter();

        void prettyPrint();
        QString errors();

        bool valid();

    // TODO: BASIC SAFETY: private:
        // QString defaultIgniterCache = "";
        // QString userIgniterCache = "";
        uint uid; // user uid who loads igniter config
        QString name, softwareName, repository, parentService, sha, shell;
        int generatedDefaultPort = 0;
        double formatVersion = 0.0;
        bool autoStart, watchPort, watchSocket, watchUdpPort, alwaysOn, resolveDomain, webApp;
        long long staticPort = -1, portsPool = -1, minimumRequiredDiskSpace = -1, configureOrder = -1, startOrder = -1, notificationLevel = -1;
        QStringList dependencies, watchHttpAddresses, domains, standaloneDependencies;
        QList<SvdScheduler*> schedulers;
        SvdShellOperations  *install = NULL,
                            *configure = NULL,
                            *start = NULL,
                            *afterStart = NULL,
                            *stop = NULL,
                            *afterStop = NULL,
                            *reload = NULL,
                            *validate = NULL,
                            *babySitter = NULL;

    private:
        char errbuf[1024];
        yajl_val nodeRoot_ = NULL, nodeDefault_ = NULL;

};



#endif

