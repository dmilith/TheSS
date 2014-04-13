/**
 *  @author dmilith
 *
 *   © 2013-2014 - VerKnowSys
 *
 */

#ifndef __DEPLOY_H__
#define __DEPLOY_H__


#include <QtCore>

#include "../globals/globals.h"
#include "../core/utils.h"
#include "../notifications/notifications.h"
#include "../core/logger.h"
#include "../core/utils.h"
#include "../service_spawner/process.h"
#include "../service_spawner/service_config.h"
#include "../death_watch/death_watch.h"
#include "webapp_types.h"


#define MAX_DEPLOYS_TO_KEEP 3 /* keep 5 deploys in releases */
#define MINIMUM_DISK_SPACE_IN_MEGS 1024 /* 1 GiB */
#define OLD_SERVICE_SHUTDOWN_TIMEOUT 30 /* in seconds */
#define DEFAULT_DEPLOYER_TIMEOUT_INTERVAL 60 /* in seconds */
#define DEFAULT_DEPLOYER_SHELL "/Software/Zsh/exports/zsh"

// const QStringList getStandaloneDeps();
QString nginxEntry(WebAppTypes type, QString latestReleaseDir, QString domain, QString serviceName, QString stage, QString port, SvdServiceConfig* svConfig, QString sslPemPath = ""); /* empty sslPemPath means, that default self-signed cert will be created for this entry */
QString getDbName(WebDatastore db);
QList<WebDatastore> detectDatastores(QString& deps, QString& depsFile);
bool validateNginxEntry(QString& servicePath, QString contents);
void generateDatastoreSetup(QList<WebDatastore> dbs, QString serviceName, QString stage, WebAppTypes appType);
void generateServicePorts(QString servicePath, int amount = 1);
void prepareSharedDirs(QString& latestReleaseDir, QString& servicePath, QString& stage);
void prepareSharedSymlinks(QString& serviceName, QString& latestReleaseDir, QString& stage, SvdServiceConfig* svConfig);
void installDependencies(QString& serviceName, QString& latestReleaseDir, QString& releaseName);
void cloneRepository(QString& serviceName, QString& branch, QString releaseName);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);
QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain);
QString buildEnv(QString& serviceName, QStringList deps, QString preEnv);
QStringList filterSpawnableDependencies(const QString& deps);
void startWithoutDependencies(const QString& servicePath);
void requestDependenciesRunningOf(const QString& serviceName, const QStringList appDependencies, const QString& releaseName);
// void requestDependenciesStoppedOf(const QString& serviceName, const QStringList appDependencies, const QString& releaseName);

QString injectPorts(const QStringList& servPorts, const QString& identifier);

#endif
