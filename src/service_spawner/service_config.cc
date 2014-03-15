/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "service_config.h"
#include "cron_entry.h"
#include "../notifications/notifications.h"


SvdScheduler::SvdScheduler(const QString& initialCronEntry, const QString& initialCommands) {
    cronEntry = initialCronEntry;
    commands = initialCommands;
    auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
    QString content = cronEntry + commands;
    hash->addData(content.toUtf8(), content.length());
    sha = hash->result().toHex();
    delete hash;
}


QStringList SvdServiceConfig::getArray(yajl_val nodeDefault, yajl_val nodeRoot, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logTrace() << "element:" << element;
    int size = input.length();
    char *path[size];
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length()];
        strncpy(path[i], s.toUtf8().constData(), s.length() + 1);
        path[s.length()] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    /*
        gather values from given json array.
     */
    QStringList buf;
    yajl_val v = yajl_tree_get(nodeDefault, (const char**)path, yajl_t_array);
    yajl_val w = yajl_tree_get(nodeRoot, (const char**)path, yajl_t_array);
    for (int j = 0; j <= i; j++) {
        delete[] path[j];
    }

    if (v and YAJL_IS_ARRAY(v)) {
        logTrace() << "Default Array:" << element;
        int len = v->u.array.len;
        for (i = 0; i < len; ++i) { /* gather default list */
            yajl_val obj = v->u.array.values[i];
            logTrace() << "Parsed Default Array value:" << YAJL_GET_STRING(obj);
            buf << YAJL_GET_STRING(obj);
        }
        logTrace() << "Gathered Default Array:" << buf;
    }
    if (w and YAJL_IS_ARRAY(w)) {
        int len = w->u.array.len;
        for (i = 0; i < len; ++i) { /* gather igniter list */
            yajl_val obj = w->u.array.values[i];
            logTrace() << "Parsed Root Array value:" << YAJL_GET_STRING(obj);
            buf << YAJL_GET_STRING(obj);
        }
        logTrace() << "Gathered Root Array:" << buf;
    }
    return buf;
}


QStringList SvdServiceConfig::getArray(const QString element) {
    return getArray(nodeDefault_, nodeRoot_, element);
}


long long SvdServiceConfig::getInteger(yajl_val nodeDefault, yajl_val nodeRoot, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logTrace() << "element:" << element;
    int size = input.length(), i = 0;
    char *path[size];
    foreach (QString s, input) {
        path[i] = new char[s.length()];
        strncpy(path[i], s.toUtf8().constData(), s.length() + 1);
        path[s.length()] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    yajl_val v = yajl_tree_get(nodeDefault, (const char**)path, yajl_t_any);
    yajl_val w = yajl_tree_get(nodeRoot, (const char**)path, yajl_t_any);
    for (int j = 0; j <= i; j++) {
        delete[] path[j];
    }

    /* user igniter has priority */
    if (w and YAJL_IS_INTEGER(w)) {
        long long lng = YAJL_GET_INTEGER(w);
        logTrace() << "Parsed Root Integer:" << QString::number(lng);
        return lng;
    }
    if (v and YAJL_IS_INTEGER(v)) {
        long long lng = YAJL_GET_INTEGER(v);
        logTrace() << "Parsed Default Integer:" << QString::number(lng);
        return lng;
    }
    logError() << "Not a integer:" << element;
    return 0;
}


long long SvdServiceConfig::getInteger(const QString element) {
    return getInteger(nodeDefault_, nodeRoot_, element);
}


bool SvdServiceConfig::getBoolean(yajl_val nodeDefault, yajl_val nodeRoot, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logTrace() << "element:" << element;
    int size = input.length();
    char *path[size];
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length()];
        strncpy(path[i], s.toUtf8().constData(), s.length() + 1);
        path[s.length()] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    yajl_val v = yajl_tree_get(nodeDefault, (const char**)path, yajl_t_any);
    yajl_val w = yajl_tree_get(nodeRoot, (const char**)path, yajl_t_any);
    for (int j = 0; j <= i; j++) {
        delete[] path[j];
    }

    if (w and (YAJL_IS_TRUE(w) or YAJL_IS_FALSE(w))) {
        if (YAJL_IS_TRUE(w)) {
            return true;
        } else {
            return false;
        }
    }
    if (v and (YAJL_IS_TRUE(v) or YAJL_IS_FALSE(v))) {
        if (YAJL_IS_TRUE(v)) {
            return true;
        } else {
            return false;
        }
    }

    logError() << "No such boolean node:" << element << "in igniter:" << name;
    return false;
}


bool SvdServiceConfig::getBoolean(const QString element) {
    return getBoolean(nodeDefault_, nodeRoot_, element);
}


QString SvdServiceConfig::getString(yajl_val nodeDefault, yajl_val nodeRoot, const QString element) {
    /* building paths */
    QStringList input = element.split("/");
    logTrace() << "element:" << element;
    int size = input.length();
    char *path[size];
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length()];
        strncpy(path[i], s.toUtf8().constData(), s.length() + 1);
        path[s.length()] = ZERO_CHAR;
        i++;
    }
    path[i] = ZERO_CHAR;

    yajl_val v = yajl_tree_get(nodeDefault, (const char**)path, yajl_t_string);
    yajl_val w = yajl_tree_get(nodeRoot, (const char**)path, yajl_t_string);
    for (int j = 0; j <= i; j++) {
        delete[] path[j];
    }

    QString finalContent = "";
    /*
        merge default source string with root source string for each String json element
     */
    if (v and YAJL_IS_STRING(v)) {
        logTrace() << "Parsed Default String:" << QString(YAJL_GET_STRING(v));
        finalContent += YAJL_GET_STRING(v);
    }
    if (w and YAJL_IS_STRING(w)) {
        logTrace() << "Parsed Root String:" << QString(YAJL_GET_STRING(w));
        finalContent += YAJL_GET_STRING(w);
    }

    if (finalContent.isEmpty())
        logError() << "No such string node:" << element << "in igniter:" << name;
    return finalContent;
}


QString SvdServiceConfig::getString(const QString element) {
    return getString(nodeDefault_, nodeRoot_, element);
}


SvdServiceConfig::SvdServiceConfig() { /* Load default values */
    name = "Default"; // must be declared first
    uid = getuid();

    char errbuf[1024];
    auto defaults = loadDefaultIgniter();
    if (defaults.isEmpty()) {
        if (defaultsCache.isEmpty()) {
            QString msg = "Igniters defaults must be always valid. Cannot continue with empty default content.";
            notification(msg, FATAL);
            return;
        } else
            defaults = defaultsCache; /* take value from cache */
    }
    nodeDefault_ = yajl_tree_parse(defaults.toUtf8().data(), errbuf, sizeof(errbuf));
    if (QString(errbuf).length() > 0) {
        logError() << "ERR:" << errbuf;
    }
    if (defaults.isEmpty() or nodeDefault_ == NULL) {
        if (defaultsCache.isEmpty()) {
            logFatal() << "Failed to load default igniter which is mandatory. Cannot continue";
            return;
        } else {
            logInfo() << "Updating igniter cache";
            defaultsCache = defaults;
        }
    }

    QString formatVersion = getString("formatVersion");
    if (not QString(APP_VERSION).contains(formatVersion)) {
        logError() << "Outdated igniter format detected. Please update your igniters!";
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

    //yajl_tree_free(node_);
    prettyPrint();
}


void SvdServiceConfig::prettyPrint() {
    logInfo() << "Service |                     name:" << name;
    logInfo() << "        |             softwareName:" << softwareName;
    logInfo() << "        |                autoStart:" << autoStart;
    logInfo() << "        |        notificationLevel:" << notificationLevel;
    logInfo() << "        |                   webApp:" << webApp;
    logInfo() << "        |                watchPort:" << watchPort;
    logInfo() << "        |             watchUdpPort:" << watchUdpPort;
    logInfo() << "        |                 alwaysOn:" << alwaysOn;
    logInfo() << "        |            resolveDomain:" << resolveDomain;
    logInfo() << "        |               staticPort:" << staticPort;
    logInfo() << "        | minimumRequiredDiskSpace:" << minimumRequiredDiskSpace;
    logInfo() << "        |                portsPool:" << portsPool;
    logInfo() << "        |               repository:" << repository;
    logInfo() << "        |            parentService:" << parentService;
    logInfo() << "        |           configureOrder:" << configureOrder;
    logInfo() << "        |               startOrder:" << startOrder;
    logInfo() << "        |             dependencies:" << dependencies;
    logInfo() << "        |   standaloneDependencies:" << standaloneDependencies;
    logInfo() << "        |                  domains:" << domains;
    logInfo() << "        |       watchHttpAddresses:" << watchHttpAddresses;
    logInfo() << "        |        install->commands:" << install->commands;
    logInfo() << "        |      configure->commands:" << configure->commands;
    logInfo() << "        |          start->commands:" << start->commands;
    logInfo() << "        |     afterStart->commands:" << afterStart->commands;
    logInfo() << "        |           stop->commands:" << stop->commands;
    logInfo() << "        |      afterStop->commands:" << afterStop->commands;
    logInfo() << "        |         reload->commands:" << reload->commands;
    logInfo() << "        |       validate->commands:" << validate->commands;
    logInfo() << "        |     babySitter->commands:" << babySitter->commands;

    return;
}


SvdServiceConfig::SvdServiceConfig(const QString& serviceName) {
    // SvdServiceConfig();
    name = serviceName; // this must be declared first!
    uid = getuid();

    char errbuf[1024];
    auto defaults = loadDefaultIgniter();
    auto root = loadIgniter();
    if (root.isEmpty()) {
        logError() << "Empty service config root for:" << serviceName;
    }
    // if (node_ != NULL)
    //     yajl_tree_free(node_);
    nodeDefault_ = yajl_tree_parse(defaults.toUtf8().constData(), errbuf, sizeof(errbuf));
    if (QString(errbuf).length() > 0) {
        logError() << "Dflt-err of:" << name << "::" << errbuf;
    }
    nodeRoot_ = yajl_tree_parse(root.toUtf8().constData(), errbuf, sizeof(errbuf));
    if (QString(errbuf).length() > 0) {
        logError() << "Root-err of:" << name << "::" << errbuf;
    }
    if (nodeDefault_ == NULL) {
        logFatal() << "Json parse failure for Default!";
    }
    if (nodeRoot_ == NULL) {
        logError() << "Json parse failure for:" << serviceName;
    }
    // if (root.isEmpty()) {
    //     if (defaultsCache.isEmpty()) {
    //         logFatal() << "Failed to load mandatory" << name << "igniter. Cannot continue.";
    //         return;
    //     } else {
    //         logInfo() << "Updating igniter cache";
    //         defaultsCache = root;
    //     }
    // }

    auto hash = new QCryptographicHash(QCryptographicHash::Sha1);
    QString content = readFileContents(userIgniter()).trimmed();
    hash->addData(content.toUtf8(), content.length());
    this->sha = hash->result().toHex();
    delete hash;

    getOrCreateDir(prefixDir() + DEFAULT_SERVICE_PORTS_DIR);
    getOrCreateDir(prefixDir() + DEFAULT_SERVICE_DOMAINS_DIR);
    getOrCreateDir(prefixDir() + DEFAULT_SERVICE_ENVS_DIR);
    getOrCreateDir(prefixDir() + DEFAULT_SERVICE_PIDS_DIR);
    getOrCreateDir(prefixDir() + DEFAULT_SERVICE_LOGS_DIR);
    getOrCreateDir(prefixDir() + DEFAULT_SERVICE_CONFS_DIR);

    portsPool = getInteger("portsPool");
    QString portsDirLocation = prefixDir() + DEFAULT_SERVICE_PORTS_DIR;
    logTrace() << "Port pool for service:" << name << "=>" << QString::number(portsPool);
    if (portsPool > 1)
        if (QDir().exists(portsDirLocation))
            for (int indx = 1; indx < portsPool; indx++) {
                QString portFilePath = QString(portsDirLocation + QString::number(indx)).trimmed();
                if (not QFile::exists(portsDirLocation + QString::number(indx))) {
                    logTrace() << "Creating port file:" << portsDirLocation + QString::number(indx);
                    uint freePort = registerFreeTcpPort();
                    writeToFile(portFilePath, QString::number(freePort));
                }
            }

    /* then replace main port */
    staticPort = getInteger("staticPort");
    QString portFilePath = portsDirLocation + DEFAULT_SERVICE_PORT_NUMBER; // getOrCreateDir
    if (staticPort != -1) { /* defined static port */
        logTrace() << "Set static port:" << staticPort << "for service" << name;
        writeToFile(portFilePath, QString::number(staticPort));
        generatedDefaultPort = QString::number(staticPort);
    } else {
        if (not QFile::exists(portFilePath)) {
            generatedDefaultPort = QString::number(registerFreeTcpPort());
            logTrace() << "Set random free port:" << generatedDefaultPort << "for service" << name;
            writeToFile(portFilePath, generatedDefaultPort);
        } else
            generatedDefaultPort = readFileContents(portFilePath).trimmed();
    }
    #ifdef QT_DEBUG
        Q_ASSERT(not generatedDefaultPort.isEmpty());
    #endif

    softwareName = getString("softwareName");
    autoStart = getBoolean("autoStart");
    notificationLevel = getInteger("notificationLevel");
    webApp = getBoolean("webApp");
    watchPort = getBoolean("watchPort");
    watchUdpPort = getBoolean("watchUdpPort");
    alwaysOn = getBoolean("alwaysOn");
    resolveDomain = getBoolean("resolveDomain");
    minimumRequiredDiskSpace = getInteger("minimumRequiredDiskSpace");
    repository = getString("repository");
    parentService = getString("parentService");
    configureOrder = getInteger("configureOrder");
    startOrder = getInteger("startOrder");

    dependencies = getArray("dependencies");
    standaloneDependencies = getArray("standaloneDeps");
    domains = getArray("domains");
    watchHttpAddresses = getArray("watchHttp");

    /* load service scheduler data */
    QStringList igniterSchedulers = getArray("schedulers");
    Q_FOREACH(QString scheduler, igniterSchedulers) {
        QString entry = scheduler.split("!").first();
        QString commands = scheduler.split("!").last();
        schedulers.push_back(
            new SvdScheduler(
                entry, replaceAllSpecialsIn(commands)
            ));
    }

    /* laod service hooks */
    install = new SvdShellOperations(replaceAllSpecialsIn(getString("install/commands")), getString("install/expectOutput"));
    configure = new SvdShellOperations(replaceAllSpecialsIn(getString("configure/commands")), getString("configure/expectOutput"));
    start = new SvdShellOperations(replaceAllSpecialsIn(getString("start/commands")), getString("start/expectOutput"));
    afterStart = new SvdShellOperations(replaceAllSpecialsIn(getString("afterStart/commands")), getString("afterStart/expectOutput"));
    stop = new SvdShellOperations(replaceAllSpecialsIn(getString("stop/commands")), getString("stop/expectOutput"));
    afterStop = new SvdShellOperations(replaceAllSpecialsIn(getString("afterStop/commands")), getString("afterStop/expectOutput"));
    reload = new SvdShellOperations(replaceAllSpecialsIn(getString("reload/commands")), getString("reload/expectOutput"));
    validate = new SvdShellOperations(replaceAllSpecialsIn(getString("validate/commands")), getString("validate/expectOutput"));
    babySitter = new SvdShellOperations(replaceAllSpecialsIn(getString("babySitter/commands")), getString("babySitter/expectOutput"));


    /* on this stage, we want to replace these igniter constants: */
    QMap<QString, QString> injectHooks;
    injectHooks["SERVICE_INSTALL_HOOK"] = install->commands;
    injectHooks["SERVICE_START_HOOK"] = start->commands;
    injectHooks["SERVICE_STOP_HOOK"] = stop->commands;
    injectHooks["SERVICE_AFTERSTART_HOOK"] = afterStart->commands;
    injectHooks["SERVICE_AFTERSTOP_HOOK"] = afterStop->commands;
    injectHooks["SERVICE_CONFIGURE_HOOK"] = configure->commands;
    injectHooks["SERVICE_BABYSITTER_HOOK"] = babySitter->commands;
    injectHooks["SERVICE_VALIDATE_HOOK"] = validate->commands;

    QList<SvdShellOperations*> operations;
    operations << reload << install << start << stop << afterStart << afterStop << configure << babySitter << validate;
    Q_FOREACH(auto operation, operations) { /* for each operation, do hook injection */
        Q_FOREACH(QString hook, injectHooks.keys()) {
            QString old_commds = operation->commands;
            operation->commands = operation->commands.replace(hook, injectHooks.value(hook));
            if (operation->commands != old_commds) {
                logTrace() << "Performing igniter injections of hook:" << hook << "in service:" << name;
                logTrace() << hook << "- injecting content:" << injectHooks.value(hook) << " - COMMANDS: " << operation->commands;
                logTrace() << "OLD value:" << old_commds;
                logTrace() << "NEW value:" << operation->commands;
            }
        }
    }

    QString serviceDataDir = getOrCreateDir(getServiceDataDir(name));
    QString autoStFile = serviceDataDir + AUTOSTART_TRIGGER_FILE;
    if (autoStart and not QFile::exists(autoStFile)) {
        logInfo() << "Autostart predefined on igniter side (has highest priority): Autostart state set for service:" << name;
        logTrace() << "Touching:" << autoStFile;
        touch(autoStFile);
    }

    prettyPrint();
}


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
    //     for (int i = 0; i < schedulers.length(); i++)
    //         delete schedulers.at(i);
    // }


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
        logTrace() << "User side igniter Defaults found, and will be used:" << userSideDefaultIgniter;
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
                            logTrace() << "Processing an address:" << value.toString();
                            userAddress = value.toString();
                            if (userAddress == DEFAULT_LOCAL_ADDRESS) {
                                logTrace() << "Ignoring localhost address for domain resolve.";
                            } else if (userAddress.contains(":")) {
                                logTrace() << "Ignoring IPV6 address:" << userAddress;
                            } else {
                                replaceWith = userAddress;
                                logTrace() << "Resolved address of domain " << domdom << " is " << userAddress;
                            }
                        }
                        /* replace address */
                        if (replaceWith.isEmpty()) {
                            logTrace() << "Fallback to local address for domain:" << domdom;
                            ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_LOCAL_ADDRESS);
                        } else {
                            logTrace() << "Final result of domain resolve is:" << replaceWith;
                            ccont = ccont.replace("SERVICE_ADDRESS", replaceWith); /* replace with user address content */
                        }
                    } else {
                        logTrace() << "Empty domain resolve of: " << domdom << "for service:" << name << "Setting local address: " << DEFAULT_LOCAL_ADDRESS;
                        ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_LOCAL_ADDRESS);
                    }
                }

            } else { /* don't resolve domain, just take first address available.. */
                auto list = info.addresses(); /* .. that's not 127.0.0.1 nor IPV6 */
                QString resultAddress = "";
                Q_FOREACH(QHostAddress value, list) {
                    logTrace() << "Processing an address:" << value.toString();
                    userAddress = value.toString();
                    if (userAddress == "127.0.0.1") {
                        logTrace() << "Ignoring localhost address.";
                    } else if (userAddress.contains(":")) {
                        logTrace() << "Ignoring IPV6 address:" << userAddress << "Doing fallback to local v4 address.";
                        resultAddress = DEFAULT_LOCAL_ADDRESS;
                    } else {
                        logInfo() << "Service:" << name << "bound to address:" << userAddress;
                        resultAddress = userAddress;
                    }
                }

                if (resultAddress.trimmed().isEmpty()) {
                    logTrace() << "No address. Doing fallback to local host address for improved security.";
                    ccont = ccont.replace("SERVICE_ADDRESS", DEFAULT_LOCAL_ADDRESS);
                } else {
                    ccont = ccont.replace("SERVICE_ADDRESS", resultAddress); /* replace with user address content */
                }

                // logTrace() << "Set address of domain " << userDomain << " as wildcard address: " << DEFAULT_WILDCARD_ADDRESS;
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
