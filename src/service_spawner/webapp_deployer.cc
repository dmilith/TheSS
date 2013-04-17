/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "webapp_deployer.h"


void SvdWebAppDeployer::detect() {
    auto appDetector = new WebAppTypeDetector(getWebAppsDir() + "/" + this->domain);
    this->appType = appDetector->getType();
    this->typeName = appDetector->typeName;
    logDebug() << "Detected application type:" << this->typeName;
    delete appDetector;
}


SvdWebAppDeployer::SvdWebAppDeployer(const QString& domain) {
    logInfo() << "Performing webapp deploy for domain:" << domain;
    this->domain = domain;
    detect();
}


QString SvdWebAppDeployer::getDomain() {
    return this->domain;
}


WebAppTypes SvdWebAppDeployer::getType() {
    return this->appType;
}


QString SvdWebAppDeployer::getTypeName() {
    return this->typeName;
}


SvdWebAppDeployer::~SvdWebAppDeployer() {}


void SvdWebAppDeployer::startSlot() {
    detect();
    logDebug() << "Invoked start slot for:" << typeName << "webapp for domain:" << domain;

}


void SvdWebAppDeployer::stopSlot() {
    detect();
    logDebug() << "Invoked stop slot for:" << typeName << "webapp for domain:" << domain;
}


void SvdWebAppDeployer::restartSlot() {
    detect();
    logDebug() << "Invoked restart slot for:" << typeName << "webapp for domain:" << domain;
    stopSlot();
    startSlot();
}


void SvdWebAppDeployer::reloadSlot() {
    detect();
    logDebug() << "Invoked reload slot for:" << typeName << "webapp for domain:" << domain;
}


