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
#include "../jsoncpp/json/json.h"
#include "utils.h"
#include "cron_entry.h"

#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QTime>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QNetworkInterface>


class SvdSchedulerAction {

    public:
        SvdSchedulerAction(const QString& initialCronEntry, const QString& initialCommands);
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
        bool serviceInstalled();
        bool serviceConfigured();

        Json::Value* loadDefaultIgniter();
        Json::Value* loadIgniter();


        uint uid; // user uid who loads igniter config
        QString name, softwareName, repository, parentService;
        bool autoStart, reportAllErrors, reportAllInfos, reportAllDebugs, watchPort, watchUdpPort, alwaysOn, resolveDomain, webApp;
        int staticPort, portsPool, minimumRequiredDiskSpace, configureOrder, startOrder;
        QStringList dependencies, watchHttpAddresses, domains;
        QList<SvdSchedulerAction*> schedulerActions;
        SvdShellOperations *install, *configure, *start, *afterStart, *stop, *afterStop, *reload, *validate, *babySitter;

};



#endif

