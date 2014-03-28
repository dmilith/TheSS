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
#include "utils.h"
#include "cron_entry.h"

#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QNetworkInterface>
#include <iostream>

#define ZERO_CHAR (char *)0
#define MAX_DEPTH 64 /* max supported json tree depth */

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

        void getTreeNode(yajl_val nodeDefault, yajl_val nodeRoot, const char* element, yajl_val* v, yajl_val* w);

        QString getString(const char* element);
        QString getString(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);

        QStringList getArray(const char* element);
        QStringList getArray(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);

        bool getBoolean(const char* element);
        bool getBoolean(const yajl_val nodeDefault, const yajl_val nodeRoot, const char* element);

        long long getInteger(const char* element);
        long long getInteger(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);

        double getDouble(const char* element);
        double getDouble(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);

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
        bool autoStart, watchPort, watchUdpPort, alwaysOn, resolveDomain, webApp;
        long long staticPort, portsPool, minimumRequiredDiskSpace, configureOrder, startOrder, notificationLevel;
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

