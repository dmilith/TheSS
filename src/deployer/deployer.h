/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __DEPLOYER_H__
#define __DEPLOYER_H__


#include "../service_spawner/utils.h"
#include "../service_spawner/process.h"
#include "webapp_types.h"

#include <QObject>


void installDependencies();
void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch, QString& stage);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);
QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain);

#endif
