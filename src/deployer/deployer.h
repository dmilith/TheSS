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
            return " \n\
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


void installDependencies();
void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch, QString& stage);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);
QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain);

#endif
