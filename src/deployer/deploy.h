/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __DEPLOY_H__
#define __DEPLOY_H__


#include <QtCore>

#include "../globals/globals.h"
#include "../service_spawner/utils.h"
#include "../notifications/notifications.h"
#include "../service_spawner/logger.h"
#include "../service_spawner/utils.h"
#include "../service_spawner/process.h"
#include "webapp_types.h"


#define MAX_DEPLOYS_TO_KEEP 3 /* keep 5 deploys in releases */
#define MINIMUM_DISK_SPACE_IN_MEGS 512
#define OLD_SERVICE_SHUTDOWN_TIMEOUT 60 /* in seconds */


const QStringList getAllowedToSpawnDeps();
QString nginxEntry(WebAppTypes type, QString latestReleaseDir, QString domain, QString serviceName, QString stage, QString port, QString sslPemPath = ""); /* empty sslPemPath means, that default self-signed cert will be created for this entry */
QString getDbName(WebDatastore db);
QList<WebDatastore> detectDatastores(QString& deps, QString& depsFile);
bool validateNginxEntry(QString& servicePath, QString contents);
void spawnBinBuild(QString& latestReleaseDir, QString& serviceName, QString& servicePath, QStringList appDependencies, QString& stage);
void generateDatastoreSetup(QList<WebDatastore> dbs, QString serviceName, QString stage, WebAppTypes appType);
void generateServicePorts(QString servicePath, int amount = 1);
void prepareSharedDirs(QString& latestReleaseDir, QString& servicePath, QString& stage);
void prepareSharedSymlinks(QString& latestReleaseDir, QString& servicePath, QString& stage);
void prepareHttpProxy(QString& servicePath, QString& appType, QString& latestReleaseDir, QString& domain, QString& serviceName, QString& stage);
void installDependencies(QString& serviceName, QString& latestReleaseDir);
void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch, QString& domain);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);
QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain);
QString buildEnv(QString& serviceName, QStringList deps);

#endif
