/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "service_config.h"
#include "cron_entry.h"
#include "../notifications/notifications.h"


SvdSchedulerAction::SvdSchedulerAction(const QString& initialCronEntry, const QString& initialCommands) {
    cronEntry = initialCronEntry;
    commands = initialCommands;
    auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
    QString content = cronEntry + commands;
    hash->addData(content.toUtf8(), content.length());
    sha = hash->result().toHex();
    delete hash;
}


SvdServiceConfig::SvdServiceConfig() { /* Load default values */
    name = "Default"; // must be declared first
    uid = getuid();
    try {
        auto defaults = loadDefaultIgniter();
        if (not defaults) {
            QString msg = "Igniters defaults must be always valid. Cannot continue.";
            notification(msg, "", FATAL);
        }
        softwareName = (*defaults)["softwareName"].asCString();
        autoStart = (*defaults)["autoStart"].asBool();
        reportAllErrors = (*defaults)["reportAllErrors"].asBool();
        reportAllInfos = (*defaults)["reportAllInfos"].asBool();
        reportAllDebugs = (*defaults)["reportAllDebugs"].asBool();
        watchPort = (*defaults)["watchPort"].asBool();
        alwaysOn = (*defaults)["alwaysOn"].asBool();
        staticPort = (*defaults)["staticPort"].asInt();
        minimumRequiredDiskSpace = (*defaults)["minimumRequiredDiskSpace"].asInt();
        domain = (*defaults)["domain"].asCString();
        portsPool = (*defaults)["portsPool"].asInt();
        repository = (*defaults)["repository"].asCString();
        parentService = (*defaults)["parentService"].asCString();

        /* load default service dependencies */
        for (uint index = 0; index < (*defaults)["dependencies"].size(); ++index ) {
            try {
                auto element = (*defaults)["dependencies"][index];
                if (element.isString())
                    dependencies.push_back(element.asCString());
                else
                    logError() << "Invalid JSON Type for dependencies. They all should be Strings";
            } catch (std::exception &e) {
                logDebug() << "Exception while parsing default dependencies of" << name;
            }
        }
        logDebug() << "Defined dependencies:" << name << "list:" << dependencies;

        /* laod service hooks */
        install = new SvdShellOperations(
            (*defaults)["install"]["commands"].asCString(),
            (*defaults)["install"]["expectOutput"].asCString());

        configure = new SvdShellOperations(
            (*defaults)["configure"]["commands"].asCString(),
            (*defaults)["configure"]["expectOutput"].asCString());

        start = new SvdShellOperations(
            (*defaults)["start"]["commands"].asCString(),
            (*defaults)["start"]["expectOutput"].asCString());

        afterStart = new SvdShellOperations(
            (*defaults)["afterStart"]["commands"].asCString(),
            (*defaults)["afterStart"]["expectOutput"].asCString());

        stop = new SvdShellOperations(
            (*defaults)["stop"]["commands"].asCString(),
            (*defaults)["stop"]["expectOutput"].asCString());

        afterStop = new SvdShellOperations(
            (*defaults)["afterStop"]["commands"].asCString(),
            (*defaults)["afterStop"]["expectOutput"].asCString());

        reload = new SvdShellOperations(
            (*defaults)["reload"]["commands"].asCString(),
            (*defaults)["reload"]["expectOutput"].asCString());

        validate = new SvdShellOperations(
            (*defaults)["validate"]["commands"].asCString(),
            (*defaults)["validate"]["expectOutput"].asCString());

        babySitter = new SvdShellOperations(
            (*defaults)["babySitter"]["commands"].asCString(),
            (*defaults)["babySitter"]["expectOutput"].asCString());

        delete defaults;

    } catch (std::exception &e) {
        QString msg = QString("Thrown Exception: ") + e.what() + " in Default service.";
        notification(msg, "", FATAL);
        exit(JSON_FORMAT_EXCEPTION_ERROR);
    }
}


SvdServiceConfig::SvdServiceConfig(const QString& serviceName) {
    name = serviceName; // this must be declared first!
    uid = getuid();
    try {
        auto defaults = loadDefaultIgniter();
        auto root = loadIgniter(); // NOTE: the question is.. how will this behave ;]
        if (not defaults) {
            QString msg = "Igniters defaults must be always valid. Cannot continue.";
            notification(msg, "", FATAL);
        }
        if (not root) {
            QString msg = "Error loading igniter for service: " + serviceName + "! Loading default igniter instead.";
            notification(msg, "", ERROR);
            root = loadDefaultIgniter();
        }
        softwareName = root->get("softwareName", (*defaults)["softwareName"]).asCString();
        autoStart = root->get("autoStart", (*defaults)["autoStart"]).asBool();
        reportAllErrors = root->get("reportAllErrors", (*defaults)["reportAllErrors"]).asBool();
        reportAllInfos = root->get("reportAllInfos", (*defaults)["reportAllInfos"]).asBool();
        reportAllDebugs = root->get("reportAllDebugs", (*defaults)["reportAllDebugs"]).asBool();
        watchPort = root->get("watchPort", (*defaults)["watchPort"]).asBool();
        alwaysOn = root->get("alwaysOn", (*defaults)["alwaysOn"]).asBool();
        staticPort = root->get("staticPort", (*defaults)["staticPort"]).asInt();
        portsPool = root->get("portsPool", (*defaults)["portsPool"]).asInt();
        minimumRequiredDiskSpace = root->get("minimumRequiredDiskSpace", (*defaults)["minimumRequiredDiskSpace"]).asInt();
        domain = root->get("domain", (*defaults)["domain"]).asCString();
        repository = root->get("repository", (*defaults)["repository"]).asCString();
        parentService = root->get("parentService", (*defaults)["parentService"]).asCString();

        /* load service dependencies data */
        for (uint index = 0; index < (*root)["dependencies"].size(); ++index ) {
            try {
                dependencies.push_back((*root)["dependencies"][index].asCString());
            } catch (std::exception &e) {
                QString msg = "Exception while parsing dependencies of service: " + name;
                notification(msg, "", ERROR);
            }
        }
        logTrace() << "Defined dependencies for igniter of:" << name << "list:" << dependencies;

        /* load service scheduler data */
        for (uint index = 0; index < (*root)["schedulerActions"].size(); ++index ) {
            try {
                auto object = (*root)["schedulerActions"][index].get("cronEntry", "0 0/10 * * * ?");
                auto object2 = (*root)["schedulerActions"][index].get("commands", "true");
                if (object.isString() and object2.isString()) {
                    schedulerActions.push_back(
                        new SvdSchedulerAction(
                            object.asCString(), replaceAllSpecialsIn(object2.asCString())
                        ));
                } else {
                    if (not object2.isString()) {
                        QString msg = "JSON Type: Array - Failed parsing scheduler actions in igniter: " + name + " field: commands";
                        notification(msg, "", ERROR);
                    } else {
                        QString msg = "JSON Type: Array - Failed parsing scheduler actions in igniter:" + name + " field: cronEntry";
                        notification(msg, "", ERROR);
                    }
                }
            } catch (std::exception &e) {
                QString msg = QString("Exception while parsing scheduler actions of service: ") + name + " - " + e.what();
                notification(msg, "", ERROR);
            }
        }

        /* laod service hooks */
        install = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["install"].get("commands", (*defaults)["install"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["install"].get("expectOutput", (*defaults)["install"]["expectOutput"]).asCString()));

        configure = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["configure"].get("commands", (*defaults)["configure"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["configure"].get("expectOutput", (*defaults)["configure"]["expectOutput"]).asCString()));

        start = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["start"].get("commands", (*defaults)["start"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["start"].get("expectOutput", (*defaults)["start"]["expectOutput"]).asCString()));

        afterStart = new SvdShellOperations(
            replaceAllSpecialsIn(
                QString((*root)["afterStart"].get("commands", Json::Value("")).asCString()) + QString((*defaults)["afterStart"]["commands"].asCString())), /* merge with value from Default */
            replaceAllSpecialsIn((*root)["afterStart"].get("expectOutput", (*defaults)["afterStart"]["expectOutput"]).asCString()));

        stop = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["stop"].get("commands", (*defaults)["stop"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["stop"].get("expectOutput", (*defaults)["stop"]["expectOutput"]).asCString()));

        afterStop = new SvdShellOperations(
            replaceAllSpecialsIn(
                QString((*root)["afterStop"].get("commands", Json::Value("")).asCString()) + QString((*defaults)["afterStop"]["commands"].asCString())), /* merge with value from Default */
            replaceAllSpecialsIn((*root)["afterStop"].get("expectOutput", (*defaults)["afterStop"]["expectOutput"]).asCString()));

        reload = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["reload"].get("commands", (*defaults)["reload"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["reload"].get("expectOutput", (*defaults)["reload"]["expectOutput"]).asCString()));

        validate = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["validate"].get("commands", (*defaults)["validate"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["validate"].get("expectOutput", (*defaults)["validate"]["expectOutput"]).asCString()));

        babySitter = new SvdShellOperations(
            replaceAllSpecialsIn((*root)["babySitter"].get("commands", (*defaults)["babySitter"]["commands"]).asCString()),
            replaceAllSpecialsIn((*root)["babySitter"].get("expectOutput", (*defaults)["babySitter"]["expectOutput"]).asCString()));

        delete defaults;
        delete root;

    } catch (std::exception &e) {
        QString msg = QString("Thrown Exception: ") + e.what() + " in " + serviceName + " service.";
        notification(msg, "", ERROR);
    }
}


/* destructor with memory free - welcome in C++ dmilith */
SvdServiceConfig::~SvdServiceConfig() {

    /* laod service hooks */
    delete install;
    delete configure;
    delete start;
    delete afterStart;
    delete stop;
    delete afterStop;
    delete reload;
    delete validate;
    delete babySitter;
    for (int i = 0; i < schedulerActions.length(); i++)
        delete schedulerActions.at(i);

}


const QString SvdServiceConfig::userServiceRoot() {
    return getenv("HOME") + QString(DEFAULT_USER_APPS_DIR) + "/" + softwareName;
}


bool SvdServiceConfig::serviceInstalled() {
    if (uid == 0)
        return QFile::exists(serviceRoot() + "/" + softwareName.toLower() + ".installed");
    else
        return QFile::exists(userServiceRoot() + "/" + softwareName.toLower() + ".installed");
}


bool SvdServiceConfig::serviceConfigured() {
    return QFile::exists(prefixDir() + "/.configured");
}


const QString SvdServiceConfig::serviceRoot() {
    return QString(SOFTWARE_DIR) + "/" + softwareName; // low prio
}


const QString SvdServiceConfig::prefixDir() {
    if (uid == 0) {
        return QString(SYSTEM_USERS_DIR) + QString(SOFTWARE_DATA_DIR) + "/" + name;
    } else {
        return getenv("HOME") + QString(SOFTWARE_DATA_DIR) + "/" + name;
    }
}


const QString SvdServiceConfig::defaultTemplateFile() {
    return QString(DEFAULTSOFTWARETEMPLATE) + QString(DEFAULTSOFTWARETEMPLATEEXT);
}


const QString SvdServiceConfig::rootIgniter() {
    return QString(SYSTEM_USERS_DIR) + QString(DEFAULTUSERIGNITERSDIR) + "/" + name + QString(DEFAULTSOFTWARETEMPLATEEXT);
}


const QString SvdServiceConfig::userIgniter() {
    return getenv("HOME") + QString(DEFAULTUSERIGNITERSDIR) + "/" + name + QString(DEFAULTSOFTWARETEMPLATEEXT);
}


const QString SvdServiceConfig::standardUserIgniter() {
    return QString(DEFAULTSOFTWARETEMPLATESDIR) + "/" + name + QString(DEFAULTSOFTWARETEMPLATEEXT);
}


/* XXX: TODO: OPTIMIZE, define cache for values explictly read from files for each service hook for each service */
const QString SvdServiceConfig::replaceAllSpecialsIn(const QString content) {
    QString ccont = content;

    if (name == QString("Default")) {
        logTrace() << "No specials in Default file.";
        return ccont;
    } else if (content.trimmed().isEmpty()) {
        logTrace() << "No content to replace specials in.";
        return ccont;
    } else {

        /* Replace SERVICE_ROOT */
        if (uid != 0) {
            logTrace() << "Service root for uid:" << uid << "should be located in:" << userServiceRoot();
            ccont = ccont.replace("SERVICE_ROOT", userServiceRoot());
        } else {
            logTrace() << "Service root for uid:" << uid << "should be located in:" << serviceRoot();
            ccont = ccont.replace("SERVICE_ROOT", serviceRoot());
        }

        /* set service repository */
        if (not repository.isEmpty())
            ccont = ccont.replace("SERVICE_REPOSITORY", repository);

        /* Replace PARENT_SERVICE_PREFIX */
        QString depsFull;
        if (uid != 0)
            depsFull = getenv("HOME") + QString(SOFTWARE_DATA_DIR) + "/" + parentService; // getOrCreateDir(
        else
            depsFull = QString(SYSTEM_USERS_DIR) + QString(SOFTWARE_DATA_DIR) + "/" + parentService; // getOrCreateDir(

        if (parentService.isEmpty()) {
            logTrace() << "No dependencies for:" << name;
            ccont = ccont.replace("PARENT_SERVICE_PREFIX", prefixDir()); /* fallback to original prefix dir */
        } else {
            ccont = ccont.replace("PARENT_SERVICE_PREFIX", depsFull);
        }

        /* Replace SERVICE_PREFIX */
        ccont = ccont.replace("SERVICE_PREFIX", prefixDir());

        /* Replace SERVICE_DOMAIN */
        QString domainFilePath = prefixDir() + QString(DEFAULT_SERVICE_DOMAIN_FILE);
        QString userDomain = QHostInfo::localHostName();
        if (not domain.isEmpty()) { /* predefined value of domain from igniter has a higher priority over dynamic one */
            userDomain = domain;
            ccont = ccont.replace("SERVICE_DOMAIN", domain);
            writeToFile(domainFilePath, domain);
        } else {
            if (not QFile::exists(domainFilePath)) { //(domain.isEmpty()) {
                ccont = ccont.replace("SERVICE_DOMAIN", userDomain); /* replace with user domain content */
                writeToFile(domainFilePath, userDomain);
            } else {
                QString aDomain = readFileContents(domainFilePath).trimmed();
                ccont = ccont.replace("SERVICE_DOMAIN", aDomain); /* replace with user domain content */
                userDomain = aDomain;
            }
        }

        /* Replace SERVICE_ADDRESS */
        QString address = QString(DEFAULT_SYSTEM_ADDRESS);
        QString userAddress = "";
        QHostInfo info;
        if (!userDomain.isEmpty()) {
            info = QHostInfo::fromName(QString(userDomain));
            if (!info.addresses().isEmpty()) {
                auto address = info.addresses().first();
                userAddress = address.toString();
                logTrace() << "Resolved address of domain " << userDomain << " is " << userAddress;
                ccont = ccont.replace("SERVICE_ADDRESS", userAddress); /* replace with user address content */
            } else {
                logWarn() << "Empty domain resolve of: " << userDomain << "Domain fallback to:" << domain << "(" << address << ") for service:" << name;
                ccont = ccont.replace("SERVICE_ADDRESS", address); /* replace with user address content */
            }
        } else {
            // logDebug() << "Filling address with default value";
            ccont = ccont.replace("SERVICE_ADDRESS", address);
        }

        /* sanity check for legacy .ports file */
        QString portsDirLocation = prefixDir() + QString(DEFAULT_SERVICE_PORTS_DIR);

        /* replace port pool first */
        logTrace() << "Port pool for service:" << name << "=>" << QString::number(portsPool);
        if (portsPool > 1)
            if (QDir().exists(portsDirLocation))
                for (int indx = 1; indx < portsPool; indx++) {
                    QString portFilePath = QString(portsDirLocation + QString::number(indx)).trimmed();
                    if (not QFile::exists(portsDirLocation + QString::number(indx))) {
                        logDebug() << "Creating port file:" << portsDirLocation + QString::number(indx);
                        uint freePort = registerFreeTcpPort();
                        writeToFile(portFilePath, QString::number(freePort));
                    }
                }

        /* then replace main port */
        QString portFilePath = portsDirLocation + DEFAULT_SERVICE_PORT_NUMBER; // getOrCreateDir
        if (staticPort != -1) { /* defined static port */
            logTrace() << "Set static port:" << staticPort << "for service" << name;
            writeToFile(portFilePath, QString::number(staticPort));
        } else {
            if (not QFile::exists(portFilePath)) {
                QString freePort = QString::number(registerFreeTcpPort());
                logTrace() << "Set random free port:" << freePort << "for service" << name;
                writeToFile(portFilePath, freePort);
            }
        }

        return ccont;
    }
}


/*
 *  Load igniter data in Json.
 */
Json::Value* SvdServiceConfig::loadDefaultIgniter() {
    QFile defaultIgniter(defaultTemplateFile()); /* try loading root igniter as second */
    if(!defaultIgniter.open(QIODevice::ReadOnly)) { /* check file access */
        logTrace() << "No file: " << defaultTemplateFile();
    } else {
        return parseJSON(defaultTemplateFile());
    }
    defaultIgniter.close();
    return new Json::Value();
}


/*
 *  Load igniter data in Json.
 */
Json::Value* SvdServiceConfig::loadIgniter() {
    // auto result = new Json::Value();
    QFile fileUser(userIgniter()); /* try loading user igniter as first */
    QFile fileStandardUser(standardUserIgniter()); /* try loading standard igniter as second */
    QFile fileRoot(rootIgniter()); /* try loading root igniter as third */

    if(!fileUser.open(QIODevice::ReadOnly)) { /* check file access */
        logTrace() << "No file: " << userIgniter();
    } else {
        fileUser.close();
        return parseJSON(userIgniter());
    }
    fileUser.close();

    /* also check standard location for igniters */
    if(!fileStandardUser.open(QIODevice::ReadOnly)) { /* check file access */
        logTrace() << "No file: " << standardUserIgniter();
    } else {
        fileStandardUser.close();
        return parseJSON(standardUserIgniter());
    }
    fileStandardUser.close();

    if(!fileRoot.open(QIODevice::ReadOnly)) {
        logTrace() << "No file: " << rootIgniter();
        fileRoot.close();
        return new Json::Value();
    }
    fileRoot.close();

    return parseJSON(rootIgniter());
}


SvdShellOperations::SvdShellOperations() {
    commands = QString();
    expectOutput = QString();
}


SvdShellOperations::SvdShellOperations(const QString& initialCommand, const QString& initialExpectOutput) {
    commands += initialCommand;
    expectOutput += initialExpectOutput;
}
