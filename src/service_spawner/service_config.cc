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


QStringList SvdServiceConfig::getArray(yajl_val node, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logDebug() << "element:" << element;
    const int size = input.length() + 1;
    const char *path[size];
    QString s = "";
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length() + 1];
        strcpy((char*)path[i], s.toUtf8().constData());
        path[s.length() + 1] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    QStringList buf = QStringList();
    yajl_val v = yajl_tree_get(node, path, yajl_t_array);
    if (v) {
        if (YAJL_IS_ARRAY(v)) {
            int len = v->u.array.len;
            for (i = 0; i < len; ++i) {
                yajl_val obj = v->u.array.values[i];
                buf << YAJL_GET_STRING(obj);
            }
            return buf;
        } else return buf;
    } else {
        logError() << "No such node:" << element;
    }
    return buf;
}


QStringList SvdServiceConfig::getArray(const QString element) {
    return getArray(node_, element);
}


long SvdServiceConfig::getInteger(yajl_val node, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logDebug() << "element:" << element;
    logDebug() << "INPUT:" << input;
    const int size = input.length();
    const char *path[size];
    QString s = "";
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length() + 1];
        strcpy((char*)path[i], s.toUtf8().constData());
        path[s.length() + 1] = ZERO_CHAR;
        i++;
    }
    // path[size] = ZERO_CHAR;

    yajl_val v = yajl_tree_get(node, path, yajl_t_any);
    if (v) {
        if (YAJL_IS_INTEGER(v)) {
            return YAJL_GET_INTEGER(v);
        } else return 0;
    } else {
        logError() << "No such node:" << element;
    }
    return 0;
}


long SvdServiceConfig::getInteger(const QString element) {
    return getInteger(node_, element);
}


bool SvdServiceConfig::getBoolean(yajl_val node, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logDebug() << "element:" << element;
    logDebug() << "INPUT:" << input;
    const int size = input.length() + 1;
    const char *path[size];
    QString s = "";
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length() + 1];
        strcpy((char*)path[i], s.toUtf8().constData());
        path[s.length() + 1] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    yajl_val v = yajl_tree_get(node, path, yajl_t_true);
    if (v) {
        if (YAJL_IS_TRUE(v) or YAJL_IS_FALSE(v)) {
            if (YAJL_IS_TRUE(v)) return true;
                else return false;
        } else return false; /* false will be default for malformed input */
    } else {
        logError() << "No such node:" << element;
    }
    return false;
}


bool SvdServiceConfig::getBoolean(const QString element) {
    return getBoolean(node_, element);
}


QString SvdServiceConfig::getString(yajl_val node, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logDebug() << "element:" << element;
    logDebug() << "INPUT:" << input;
    const int size = input.length() + 1;
    const char *path[size];
    QString s = "";
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length() + 1];
        strcpy((char*)path[i], s.toUtf8().constData());
        path[s.length() + 1] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    // for (int j = 0; j < i; j++)
    //     logDebug() << "Path:" << j << "-" << path[j];

    yajl_val v = yajl_tree_get(node, path, yajl_t_string);
    if (v) {
        if (YAJL_IS_STRING(v)) {
            // yajl_tree_free(v);
            logDebug() << "Parsed String:" << YAJL_GET_STRING(v);
            // delete *path;
            return YAJL_GET_STRING(v);
        } else {
            // yajl_tree_free(v);
            return "";
        }
    } else {
        // logDebug() << errno;
            // logDebug() << "PATH" << YAJL_GET_STRING(path[a]);
        logError() << "Error!";
        // }
    }
    return "";
}


QString SvdServiceConfig::getString(const QString element) {
    return getString(node_, element);
}


SvdServiceConfig::SvdServiceConfig() { /* Load default values */
    name = "Default"; // must be declared first
    uid = getuid();

    char errbuf[1024];
    auto defaults = loadDefaultIgniter();
    // if (defaults.isEmpty()) {
    //     if (defaultsCache.isEmpty()) {
    //         QString msg = "Igniters defaults must be always valid. Cannot continue with empty default content.";
    //         notification(msg, FATAL);
    //         return;
    //     } else
    //         defaults = defaultsCache; /* take value from cache */
    // }
    node_ = yajl_tree_parse(defaults.toUtf8().constData(), errbuf, sizeof(errbuf));
    if (defaults.isEmpty() or node_ == NULL) {
        if (defaultsCache.isEmpty()) {
            logError() << errbuf;
            logFatal() << "Failed to load default igniter which is mandatory. Cannot continue";
            return;
        } else {
            logInfo() << "Updating igniter cache";
            defaultsCache = defaults;
        }
    }

    softwareName = getString("softwareName");
    autoStart = getBoolean("autoStart");
    notificationLevel = getInteger("notificationLevel");
    webApp = getBoolean("webApp");
    watchPort = getBoolean("watchPort");
    watchUdpPort = getBoolean("watchUdpPort");
    alwaysOn = getBoolean("alwaysOn");
    resolveDomain = getBoolean("resolveDomain");
    staticPort = getInteger("staticPort");
    minimumRequiredDiskSpace = getInteger("minimumRequiredDiskSpace");
    portsPool = getInteger("portsPool");
    repository = getString("repository");
    parentService = getString("parentService");
    configureOrder = getInteger("configureOrder");
    startOrder = getInteger("startOrder");

    dependencies = getArray("dependencies");
    standaloneDependencies = getArray("standaloneDeps");
    domains = getArray("domains");
    watchHttpAddresses = getArray("watchHttp");

    /* laod service hooks */
    install = new SvdShellOperations(getString("install/commands"), getString("install/expectOutput"));
    configure = new SvdShellOperations(getString("configure/commands"), getString("configure/expectOutput"));
    start = new SvdShellOperations(getString("start/commands"), getString("start/expectOutput"));
    afterStart = new SvdShellOperations(getString("afterStart/commands"), getString("afterStart/expectOutput"));
    stop = new SvdShellOperations(getString("stop/commands"), getString("stop/expectOutput"));
    afterStop = new SvdShellOperations(getString("afterStop/commands"), getString("afterStop/expectOutput"));
    reload = new SvdShellOperations(getString("reload/commands"), getString("reload/expectOutput"));
    validate = new SvdShellOperations(getString("validate/commands"), getString("validate/expectOutput"));
    babySitter = new SvdShellOperations(getString("babySitter/commands"), getString("babySitter/expectOutput"));

}


SvdServiceConfig::SvdServiceConfig(const QString& serviceName) {
    SvdServiceConfig();
//     name = serviceName; // this must be declared first!
//     uid = getuid();
//     try {
//         auto defaults = loadDefaultIgniter();
//         auto root = loadIgniter(); // NOTE: the question is.. how will this behave ;]

//         auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
//         QString content = readFileContents(userIgniter()).trimmed();
//         hash->addData(content.toUtf8(), content.length());
//         this->sha = hash->result().toHex();
//         delete hash;

//         getOrCreateDir(prefixDir() + DEFAULT_SERVICE_PORTS_DIR);
//         getOrCreateDir(prefixDir() + DEFAULT_SERVICE_DOMAINS_DIR);
//         getOrCreateDir(prefixDir() + DEFAULT_SERVICE_ENVS_DIR);
//         getOrCreateDir(prefixDir() + DEFAULT_SERVICE_PIDS_DIR);
//         getOrCreateDir(prefixDir() + DEFAULT_SERVICE_LOGS_DIR);
//         getOrCreateDir(prefixDir() + DEFAULT_SERVICE_CONFS_DIR);

//         /* replace port pool first */
//         portsPool = root->get("portsPool", (*defaults)["portsPool"]).asInt();
//         QString portsDirLocation = prefixDir() + DEFAULT_SERVICE_PORTS_DIR;
//         logTrace() << "Port pool for service:" << name << "=>" << QString::number(portsPool);
//         if (portsPool > 1)
//             if (QDir().exists(portsDirLocation))
//                 for (int indx = 1; indx < portsPool; indx++) {
//                     QString portFilePath = QString(portsDirLocation + QString::number(indx)).trimmed();
//                     if (not QFile::exists(portsDirLocation + QString::number(indx))) {
//                         logDebug() << "Creating port file:" << portsDirLocation + QString::number(indx);
//                         uint freePort = registerFreeTcpPort();
//                         writeToFile(portFilePath, QString::number(freePort));
//                     }
//                 }

//         /* then replace main port */
//         staticPort = root->get("staticPort", (*defaults)["staticPort"]).asInt();
//         QString portFilePath = portsDirLocation + DEFAULT_SERVICE_PORT_NUMBER; // getOrCreateDir
//         if (staticPort != -1) { /* defined static port */
//             logTrace() << "Set static port:" << staticPort << "for service" << name;
//             writeToFile(portFilePath, QString::number(staticPort));
//             generatedDefaultPort = QString::number(staticPort);
//         } else {
//             if (not QFile::exists(portFilePath)) {
//                 generatedDefaultPort = QString::number(registerFreeTcpPort());
//                 logTrace() << "Set random free port:" << generatedDefaultPort << "for service" << name;
//                 writeToFile(portFilePath, generatedDefaultPort);
//             } else
//                 generatedDefaultPort = readFileContents(portFilePath).trimmed();
//         }
//         Q_ASSERT(not generatedDefaultPort.isEmpty()); /* XXX: temporary, but it's that important */

//         if (not defaults) {
//             QString msg = "Igniters defaults must be always valid. Cannot continue.";
//             notification(msg, FATAL);
//         }
//         if (not root) {
//             QString msg = "Error loading igniter for service: " + serviceName + "! Loading default igniter instead.";
//             notification(msg, ERROR);
//             root = loadDefaultIgniter();
//             return;
//         }
//         softwareName = root->get("softwareName", (*defaults)["softwareName"]).asCString();
//         autoStart = root->get("autoStart", (*defaults)["autoStart"]).asBool();
//         notificationLevel = root->get("notificationLevel", (*defaults)["notificationLevel"]).asInt();
//         webApp = root->get("webApp", (*defaults)["webApp"]).asBool();
//         watchPort = root->get("watchPort", (*defaults)["watchPort"]).asBool();
//         watchUdpPort = root->get("watchUdpPort", (*defaults)["watchUdpPort"]).asBool();
//         alwaysOn = root->get("alwaysOn", (*defaults)["alwaysOn"]).asBool();
//         resolveDomain = root->get("resolveDomain", (*defaults)["resolveDomain"]).asBool();
//         minimumRequiredDiskSpace = root->get("minimumRequiredDiskSpace", (*defaults)["minimumRequiredDiskSpace"]).asInt();
//         repository = root->get("repository", (*defaults)["repository"]).asCString();
//         parentService = root->get("parentService", (*defaults)["parentService"]).asCString();
//         configureOrder = root->get("configureOrder", (*defaults)["configureOrder"]).asInt();
//         startOrder = root->get("startOrder", (*defaults)["startOrder"]).asInt();

//         /* load predefined standalone dependencies */
//         for (uint index = 0; index < (*root)["standaloneDeps"].size(); ++index ) {
//             try {
//                 auto element = (*root)["standaloneDeps"][index];
//                 if (element.isString())
//                     standaloneDependencies.push_back(element.asCString());
//                 else
//                     logError() << "Invalid JSON Type for standaloneDependencies. They all should be Strings";
//             } catch (std::exception &e) {
//                 logDebug() << "Exception while parsing default standaloneDependencies of" << name;
//             }
//         }
//         logDebug() << "Defined standaloneDependencies:" << name << "list:" << standaloneDependencies;

//         /* load default service domains */
//         for (uint index = 0; index < (*root)["domains"].size(); ++index ) {
//             try {
//                 auto element = (*root)["domains"][index];
//                 if (element.isString())
//                     domains.push_back(element.asCString());
//                 else
//                     logError() << "Invalid JSON Type for domains. They all should be Strings";
//             } catch (std::exception &e) {
//                 logDebug() << "Exception while parsing default domains of" << name;
//             }
//         }
//         logDebug() << "Defined domains:" << name << "list:" << domains;

//         /* load http addresses to check */
//         for (uint index = 0; index < (*root)["watchHttp"].size(); ++index ) {
//             try {
//                 auto element = (*root)["watchHttp"][index];
//                 if (element.isString())
//                     watchHttpAddresses.push_back(element.asCString());
//                 else
//                     logError() << "Invalid JSON Type for watchHttpAddresses list. They all should be http url strings";
//             } catch (std::exception &e) {
//                 logDebug() << "Exception while parsing default watchHttpAddresses of" << name;
//             }
//         }

//         /* load service dependencies data */
//         for (uint index = 0; index < (*root)["dependencies"].size(); ++index ) {
//             try {
//                 dependencies.push_back((*root)["dependencies"][index].asCString());
//             } catch (std::exception &e) {
//                 QString msg = "Exception while parsing dependencies of service: " + name;
//                 notification(msg, ERROR);
//             }
//         }
//         logTrace() << "Defined dependencies for igniter of:" << name << "list:" << dependencies;

//         /* load service scheduler data */
//         for (uint index = 0; index < (*root)["schedulerActions"].size(); ++index ) {
//             try {
//                 auto object = (*root)["schedulerActions"][index].get("cronEntry", DEFAULT_CRON_ENTRY);
//                 auto object2 = (*root)["schedulerActions"][index].get("commands", "true");
//                 if (object.isString() and object2.isString()) {
//                     schedulerActions.push_back(
//                         new SvdSchedulerAction(
//                             object.asCString(), replaceAllSpecialsIn(object2.asCString())
//                         ));
//                 } else {
//                     if (not object2.isString()) {
//                         QString msg = "JSON Type: Array - Failed parsing scheduler actions in igniter: " + name + " field: commands";
//                         notification(msg, ERROR);
//                     } else {
//                         QString msg = "JSON Type: Array - Failed parsing scheduler actions in igniter:" + name + " field: cronEntry";
//                         notification(msg, ERROR);
//                     }
//                 }
//             } catch (std::exception &e) {
//                 QString msg = QString("Exception while parsing scheduler actions of service: ") + name + " - " + e.what();
//                 notification(msg, ERROR);
//             }
//         }

//         /* laod service hooks */
//         install = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["install"].get("commands", (*defaults)["install"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["install"].get("expectOutput", (*defaults)["install"]["expectOutput"]).asCString()));

//         configure = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["configure"].get("commands", (*defaults)["configure"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["configure"].get("expectOutput", (*defaults)["configure"]["expectOutput"]).asCString()));

//         start = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["start"].get("commands", (*defaults)["start"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["start"].get("expectOutput", (*defaults)["start"]["expectOutput"]).asCString()));

//         afterStart = new SvdShellOperations(
//             replaceAllSpecialsIn(
//                 QString((*root)["afterStart"].get("commands", Json::Value("")).asCString()) + QString((*defaults)["afterStart"]["commands"].asCString())), /* merge with value from Default */
//             replaceAllSpecialsIn((*root)["afterStart"].get("expectOutput", (*defaults)["afterStart"]["expectOutput"]).asCString()));

//         stop = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["stop"].get("commands", (*defaults)["stop"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["stop"].get("expectOutput", (*defaults)["stop"]["expectOutput"]).asCString()));

//         afterStop = new SvdShellOperations(
//             replaceAllSpecialsIn(
//                 QString((*root)["afterStop"].get("commands", Json::Value("")).asCString()) + QString((*defaults)["afterStop"]["commands"].asCString())), /* merge with value from Default */
//             replaceAllSpecialsIn((*root)["afterStop"].get("expectOutput", (*defaults)["afterStop"]["expectOutput"]).asCString()));

//         reload = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["reload"].get("commands", (*defaults)["reload"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["reload"].get("expectOutput", (*defaults)["reload"]["expectOutput"]).asCString()));

//         validate = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["validate"].get("commands", (*defaults)["validate"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["validate"].get("expectOutput", (*defaults)["validate"]["expectOutput"]).asCString()));

//         babySitter = new SvdShellOperations(
//             replaceAllSpecialsIn((*root)["babySitter"].get("commands", (*defaults)["babySitter"]["commands"]).asCString()),
//             replaceAllSpecialsIn((*root)["babySitter"].get("expectOutput", (*defaults)["babySitter"]["expectOutput"]).asCString()));


//         /* on this stage, we want to replace these igniter constants: */
//         QMap<QString, QString> injectHooks;
//         injectHooks["SERVICE_INSTALL_HOOK"] = install->commands;
//         injectHooks["SERVICE_START_HOOK"] = start->commands;
//         injectHooks["SERVICE_STOP_HOOK"] = stop->commands;
//         injectHooks["SERVICE_AFTERSTART_HOOK"] = afterStart->commands;
//         injectHooks["SERVICE_AFTERSTOP_HOOK"] = afterStop->commands;
//         injectHooks["SERVICE_CONFIGURE_HOOK"] = configure->commands;
//         injectHooks["SERVICE_BABYSITTER_HOOK"] = babySitter->commands;
//         injectHooks["SERVICE_VALIDATE_HOOK"] = validate->commands;

//         QList<SvdShellOperations*> operations;
//         operations << install << start << stop << afterStart << afterStop << configure << babySitter << validate;
//         Q_FOREACH(auto operation, operations) { /* for each operation, do hook injection */
//             Q_FOREACH(QString hook, injectHooks.keys()) {
//                 QString old_commds = operation->commands;
//                 operation->commands = operation->commands.replace(hook, injectHooks.value(hook));
//                 if (operation->commands != old_commds) {
//                     logDebug() << "Performing igniter injections of hook:" << hook << "in service:" << name;
//                     logTrace() << hook << "- injecting content:" << injectHooks.value(hook) << " - COMMANDS: " << operation->commands;
//                     logDebug() << "OLD value:" << old_commds;
//                     logDebug() << "NEW value:" << operation->commands;
//                 }
//             }
//         }

//         QString serviceDataDir = getOrCreateDir(getServiceDataDir(name));
//         QString autoStFile = serviceDataDir + AUTOSTART_TRIGGER_FILE;
//         if (autoStart and not QFile::exists(autoStFile)) {
//             logInfo() << "Autostart predefined on igniter side (has highest priority): Autostart state set for service:" << name;
//             logDebug() << "Touching:" << autoStFile;
//             touch(autoStFile);
//         }

//         delete defaults;
//         delete root;

//     } catch (std::exception &e) {
//         QString msg = QString("Thrown Exception: ") + e.what() + " in " + serviceName + " service.";
//         notification(msg, ERROR);
//     }
// }


/* destructor with memory free - welcome in C++ dmilith */
// SvdServiceConfig::~SvdServiceConfig() {

//     /* laod service hooks */
//     delete install;
//     delete configure;
//     delete start;
//     delete afterStart;
//     delete stop;
//     delete afterStop;
//     delete reload;
//     delete validate;
//     delete babySitter;
//     for (int i = 0; i < schedulerActions.length(); i++)
//         delete schedulerActions.at(i);

}


const QString SvdServiceConfig::serviceRoot() {
    return QString(SOFTWARE_DIR) + softwareName; // low prio
}


const QString SvdServiceConfig::userServiceRoot() {
    return QString(getenv("HOME")) + DEFAULT_USER_APPS_DIR + "/" + softwareName;
}


bool SvdServiceConfig::serviceInstalled() { /* XXX: it's not working properly.. and even can't for now. For services with != name than original software this function is useless.. TBR */
    return QFile::exists(serviceRoot() + "/" + softwareName.toLower() + DEFAULT_SERVICE_INSTALLED_EXT) or
        QFile::exists(userServiceRoot() + "/" + softwareName.toLower() + DEFAULT_SERVICE_INSTALLED_EXT);
}


bool SvdServiceConfig::serviceConfigured() {
    return QFile::exists(prefixDir() + DEFAULT_SERVICE_CONFS_DIR + releaseName() + DEFAULT_SERVICE_CONFIGURED_FILE);
}


const QString SvdServiceConfig::prefixDir() {
    if (uid == 0) {
        return QString(SYSTEM_USERS_DIR) + SOFTWARE_DATA_DIR + name;
    } else {
        return QString(getenv("HOME")) + SOFTWARE_DATA_DIR + name;
    }
}


const QString SvdServiceConfig::releaseName() {
    return sha.right(20) + "." + generatedDefaultPort; /* that's enough (?) */
}


const QString SvdServiceConfig::defaultTemplateFile() {
    /* pick of two possible locations: /SystemUsers/Igniters and ~/Igniters */

    /* try user side defaults first */
    QString userSideDefaultIgniter = QString(getenv("HOME")) + "/Igniters/Default" + QString(DEFAULT_SOFTWARE_TEMPLATE_EXT);
    if (QFile::exists(userSideDefaultIgniter)) {
        logDebug() << "User side igniter Defaults found, and will be used:" << userSideDefaultIgniter;
        return userSideDefaultIgniter;

    } else {
        /* then, try system wide defaults */
        QString rootSideDefaultIgniter = QString(DEFAULT_SOFTWARE_TEMPLATE) + QString(DEFAULT_SOFTWARE_TEMPLATE_EXT);
        if (QFile::exists(rootSideDefaultIgniter)) {
            return rootSideDefaultIgniter;
        } else {
            logFatal() << "Default igniter wasn't found. TheSS cannot continue.";
            return "";
        }
    }
}


const QString SvdServiceConfig::rootIgniter() {
    return QString(SYSTEM_USERS_DIR) + QString(DEFAULT_USER_IGNITERS_DIR) + "/" + name + QString(DEFAULT_SOFTWARE_TEMPLATE_EXT);
}


const QString SvdServiceConfig::userIgniter() {
    return getenv("HOME") + QString(DEFAULT_USER_IGNITERS_DIR) + "/" + name + QString(DEFAULT_SOFTWARE_TEMPLATE_EXT);
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

        /* Replace SERVICE_PORT */
        ccont = ccont.replace("SERVICE_PORT", generatedDefaultPort);
        for (int indx = 1; indx < portsPool; indx++) {
            QString additionalPort = prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/" + QString::number(indx);
            logWarn() << "Defined additional port:" << additionalPort;
            ccont = ccont.replace("SERVICE_PORT" + QString::number(indx), readFileContents(additionalPort).trimmed());
        }

        /* Replace SERVICE_PREFIX */
        ccont = ccont.replace("SERVICE_PREFIX", prefixDir());

        /* Replace SERVICE_RELEASE and related values */
        ccont = ccont.replace("SERVICE_RELEASE", releaseName());

        // todo: add SERVICE_CONFIG,
        ccont = ccont.replace("SERVICE_CONF", getOrCreateDir(prefixDir() + DEFAULT_SERVICE_CONFS_DIR + releaseName()) + DEFAULT_SERVICE_CONF_FILE);
        ccont = ccont.replace("SERVICE_LOG", getOrCreateDir(prefixDir() + DEFAULT_SERVICE_LOGS_DIR + releaseName()) + DEFAULT_SERVICE_LOG_FILE);
        ccont = ccont.replace("SERVICE_ENV", getOrCreateDir(prefixDir() + DEFAULT_SERVICE_ENVS_DIR + releaseName()) + DEFAULT_SERVICE_ENV_FILE);
        ccont = ccont.replace("SERVICE_PID", getOrCreateDir(prefixDir() + DEFAULT_SERVICE_PIDS_DIR + releaseName()) + DEFAULT_SERVICE_PID_FILE);
        ccont = ccont.replace("SERVICE_SOCK", prefixDir() + DEFAULT_SERVICE_SOCKET_FILE);

        /* Replace SERVICE_DOMAIN */
        QStringList userDomains; // QHostInfo::localHostName();
        QString domainFilePath = prefixDir() + DEFAULT_SERVICE_DOMAINS_DIR;

        /* touch igniter domains */
        Q_FOREACH(auto dom, domains)
            touch(domainFilePath + dom);

        /* check domain files */
        auto fileDomains = QDir(domainFilePath).entryList(QDir::Files | QDir::NoDotAndDotDot);
        if (fileDomains.isEmpty()) {
            userDomains << DEFAULT_SYSTEM_DOMAIN; /* localhost */
            touch(domainFilePath + userDomains.first());
            ccont = ccont.replace("SERVICE_DOMAIN", userDomains.first()); /* replace with first user domain */
        } else {
            /* if any domains found, but not prefer localhost if it's already there.. */
            if (fileDomains.length() > 1) { /* more than just default localhost */
                fileDomains.removeAll(DEFAULT_LOCAL_ADDRESS);
                QFile::remove(domainFilePath + DEFAULT_SYSTEM_DOMAIN);
            }
            Q_FOREACH(QString domFile, fileDomains) {
                userDomains << domFile;
                touch(domainFilePath + domFile);
            }
            ccont = ccont.replace("SERVICE_DOMAIN", userDomains.first()); /* replace with first user domain elem */
        }

        /* Replace SERVICE_ADDRESS */
        QString address = QString(DEFAULT_LOCAL_ADDRESS);
        QString userAddress = "";
        QHostInfo info;
        if (not userDomains.isEmpty()) {
            if (resolveDomain) { /* by default domain resolve is done for each domain given by user */
                Q_FOREACH(auto domdom, userDomains) {
                    info = QHostInfo::fromName(domdom);
                    if (!info.addresses().isEmpty()) {
                        auto list = info.addresses();
                        QString replaceWith = "";
                        Q_FOREACH(QHostAddress value, list) {
                            logDebug() << "Processing an address:" << value.toString();
                            userAddress = value.toString();
                            if (userAddress == DEFAULT_LOCAL_ADDRESS) {
                                logDebug() << "Ignoring localhost address for domain resolve.";
                            } else if (userAddress.contains(":")) {
                                logDebug() << "Ignoring IPV6 address:" << userAddress;
                            } else {
                                replaceWith = userAddress;
                                logDebug() << "Resolved address of domain " << domdom << " is " << userAddress;
                            }
                        }
                        /* replace address */
                        if (replaceWith.isEmpty()) {
                            logDebug() << "Fallback to local address for domain:" << domdom;
                            ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_LOCAL_ADDRESS);
                        } else {
                            logDebug() << "Final result of domain resolve is:" << replaceWith;
                            ccont = ccont.replace("SERVICE_ADDRESS", replaceWith); /* replace with user address content */
                        }
                    } else {
                        logDebug() << "Empty domain resolve of: " << domdom << "for service:" << name << "Setting local address: " << DEFAULT_LOCAL_ADDRESS;
                        ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_LOCAL_ADDRESS);
                    }
                }

            } else { /* don't resolve domain, just take first address available.. */
                auto list = info.addresses(); /* .. that's not 127.0.0.1 nor IPV6 */
                QString resultAddress = "";
                Q_FOREACH(QHostAddress value, list) {
                    logDebug() << "Processing an address:" << value.toString();
                    userAddress = value.toString();
                    if (userAddress == "127.0.0.1") {
                        logDebug() << "Ignoring localhost address.";
                    } else if (userAddress.contains(":")) {
                        logDebug() << "Ignoring IPV6 address:" << userAddress << "Doing fallback to local v4 address.";
                        resultAddress = DEFAULT_LOCAL_ADDRESS;
                    } else {
                        logInfo() << "Service:" << name << "bound to address:" << userAddress;
                        resultAddress = userAddress;
                    }
                }

                if (resultAddress.trimmed().isEmpty()) {
                    logDebug() << "No address. Doing fallback to local host address for improved security.";
                    ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_LOCAL_ADDRESS);
                } else {
                    ccont = ccont.replace("SERVICE_ADDRESS", resultAddress); /* replace with user address content */
                }

                // logDebug() << "Set address of domain " << userDomain << " as wildcard address: " << DEFAULT_WILDCARD_ADDRESS;
                // ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_WILDCARD_ADDRESS); /* replace with user address content */
            }
        } else {
            ccont = ccont.replace("SERVICE_ADDRESS", address);
        }

        return ccont;
    }
}


/*
 *  Load igniter data in Json.
 */
QString SvdServiceConfig::loadDefaultIgniter() {
    QFile defaultIgniter(defaultTemplateFile()); /* try loading root igniter as second */
    if(!defaultIgniter.open(QIODevice::ReadOnly)) { /* check file access */
        logError() << "NO loadDefaultIgniter: " << defaultTemplateFile();
    } else {
        return readFileContents(defaultTemplateFile());
    }
    defaultIgniter.close();
    return "";
}


/*
 *  Load igniter data in Json.
 */
QString SvdServiceConfig::loadIgniter() {
    // auto result = new Json::Value();
    QFile fileUser(userIgniter()); /* try loading user igniter as first */
    QFile fileRoot(rootIgniter()); /* try loading root igniter as third */

    if(!fileUser.open(QIODevice::ReadOnly)) { /* check file access */
        logTrace() << "No file: " << userIgniter();
    } else {
        fileUser.close();
        return readFileContents(userIgniter());
    }
    fileUser.close();

    if(!fileRoot.open(QIODevice::ReadOnly)) {
        logTrace() << "No file: " << rootIgniter();
        fileRoot.close();
        return "";
    }
    fileRoot.close();

    return readFileContents(rootIgniter());
}


SvdShellOperations::SvdShellOperations() {
    commands = QString();
    expectOutput = QString();
}


SvdShellOperations::SvdShellOperations(const QString& initialCommand, const QString& initialExpectOutput) {
    commands += initialCommand;
    expectOutput += initialExpectOutput;
}
