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


inline QString nginxEntry(WebAppTypes type, QString latestReleaseDir, QString domain, QString serviceName, QString stage, QString port) {
    switch (type) {
        case StaticSite:
            return "\n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    access_log off; \n\
    location / { \n\
        index index.html index.htm; \n\
        expires 30d; \n\
    } \n\
}\n";

        case RubySite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server 127.0.0.1:" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header Host $http_host; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
    } \n\
} \n";


        case NodeSite:
            return ""; // NOTE: NYI


        case NoType:
            return "";

    }
}


inline QString nginxEntry(WebAppTypes type, QString latestReleaseDir) {
    return nginxEntry(type, latestReleaseDir, "domain", "serviceName", "stage", "port"); // NOTE: irrelevant for static apps
}


inline QString databaseYmlEntry(WebDatabase db, QString stage, QString databaseName) {
    switch (db) {

        case Postgresql:
            return stage + ": \n\
  adapter: postgresql \n\
  encoding: unicode \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  pool: 5 \n\
  port: <%= File.read(ENV['HOME'] + \"/SoftwareData/Postgresql/.ports/0\") %> \n\
  host: <%= ENV['HOME'] + \"/SoftwareData/Postgresql/\" %> \n\
"; // XXX: should contains latestRelease cause of potential database failure that might happen after db:migrate

        case Mysql:
            return ""; // NOTE: NYI

        case NoDB:
            return "";

    }
}


QString getDbName(WebDatabase db) {
    switch (db) {
        case Postgresql: return "Postgresql";
        case Mysql: return "Mysql";
        case NoDB: return "NoDB";
    }
}

void installDependencies();
void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch, QString& stage);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);
QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain);

#endif
