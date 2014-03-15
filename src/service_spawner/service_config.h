/**
 *  @author dmilith
 *
 *   Software config loader for json igniters.
 *   © 2013 - VerKnowSys
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
        SvdServiceConfig(const QString& serviceName);
        ~SvdServiceConfig(); /* free allocated objects */

        const QString replaceAllSpecialsIn(const QString content);
        const QString userServiceRoot();
        const QString serviceRoot();
        const QString prefixDir();
        const QString defaultTemplateFile();
        const QString rootIgniter();
        const QString userIgniter();
        const QString releaseName();

        QString getString(const QString element);
        QString getString(yajl_val nodeDefault, yajl_val nodeRoot, const QString element);

        QStringList getArray(const QString element);
        QStringList getArray(yajl_val nodeDefault, yajl_val nodeRoot, const QString element);

        bool getBoolean(const QString element);
        bool getBoolean(yajl_val nodeDefault, yajl_val nodeRoot, const QString element);

        long long getInteger(const QString element);
        long long getInteger(yajl_val nodeDefault, yajl_val nodeRoot, const QString element);

        bool serviceInstalled();
        bool serviceConfigured();

        QString loadDefaultIgniter();
        QString loadIgniter();

        void prettyPrint();

        QString defaultsCache = "";
        yajl_val nodeRoot_ = NULL, nodeDefault_ = NULL;
        QString jsonContent_ = "";
        uint uid; // user uid who loads igniter config
        QString name, softwareName, repository, parentService, sha, generatedDefaultPort;
        bool autoStart, watchPort, watchUdpPort, alwaysOn, resolveDomain, webApp;
        int staticPort, portsPool, minimumRequiredDiskSpace, configureOrder, startOrder, notificationLevel;
        QStringList dependencies, watchHttpAddresses, domains, standaloneDependencies;
        QList<SvdScheduler*> schedulers;
        SvdShellOperations *install, *configure, *start, *afterStart, *stop, *afterStop, *reload, *validate, *babySitter;

};



#endif

