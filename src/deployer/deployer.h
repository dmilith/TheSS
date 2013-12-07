/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __DEPLOYER_H__
#define __DEPLOYER_H__


// #include "../service_spawner/file_events_manager.h"
// #include "../service_spawner/service_config.h"
// #include "../service_spawner/service_watcher.h"
// #include "../jsoncpp/json/json.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/utils.h"
#include "../service_spawner/service.h"
#include "../service_spawner/process.h"
#include "webapp_types.h"
// #include "../service_spawner/service.h"
// #include "../service_spawner/data_collector.h"

#include <QObject>
// #include <QFile>
// #include <QEventLoop>


void installDependencies();
void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch, QString& stage);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);

#endif
