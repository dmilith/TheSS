#include <fcntl.h>
#include "TestLibrary.h"


/* test utilities */

void writeSampleOf(const char* sample, const char* file) {
    int lfp = open(file, O_RDWR | O_CREAT, 0600);
    write(lfp, sample, strlen(sample));
    close(lfp);
}

/* eof test utilities */


TestLibrary::~TestLibrary() {
    delete consoleAppender;
}


TestLibrary::TestLibrary() {
    /* Logger setup */
    consoleAppender = new ConsoleAppender();
    consoleAppender->setFormat("%t{dd-HH:mm:ss} [%-7l] <%c> %m\n");
    consoleAppender->setDetailsLevel(Logger::Debug);
    Logger::registerAppender(consoleAppender);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));

    if (not QDir().exists(getenv("HOME") + QString("/..") + DEFAULTSOFTWARETEMPLATESDIR)) {
        cout << "Install igniters first before launching test!" << endl;
        exit(1);
    }
    testDataDir = QDir::currentPath() + "/basesystem/universal/TestData";
    testDataDir2 = QDir::currentPath() + "/../basesystem/universal/TestData";
}

/* test functions */

void TestLibrary::testParseJSONRedis() {
    auto config = new SvdServiceConfig("Redis"); /* Load app specific values */
    QCOMPARE(config->name, QString("Redis"));
    QCOMPARE(config->softwareName, QString("Redis"));
    QCOMPARE(config->staticPort, -1);

    QCOMPARE(config->uid, getuid());

    QVERIFY(config->schedulerActions->first()->cronEntry.contains("*"));
    logDebug() << config->schedulerActions->first()->cronEntry;
    logDebug() << config->schedulerActions->first()->commands;

    /* verify replaceAllIn result, should not contain SERVICE_PORT, SERVICE_DOMAIN, SERVICE_ROOT, SERVICE_ADDRESS */
    QVERIFY(!config->install->commands.contains("SERVICE_PORT"));
    QVERIFY(!config->start->commands.contains("SERVICE_PORT"));
    QVERIFY(!config->configure->commands.contains("SERVICE_PORT"));
    QVERIFY(!config->afterStart->commands.contains("SERVICE_PORT"));

    QVERIFY(!config->install->commands.contains("SERVICE_ROOT"));
    QVERIFY(!config->start->commands.contains("SERVICE_ROOT"));
    QVERIFY(!config->configure->commands.contains("SERVICE_ROOT"));
    QVERIFY(!config->afterStart->commands.contains("SERVICE_ROOT"));

    QVERIFY(!config->install->commands.contains("SERVICE_DOMAIN"));
    QVERIFY(!config->start->commands.contains("SERVICE_DOMAIN"));
    QVERIFY(!config->configure->commands.contains("SERVICE_DOMAIN"));
    QVERIFY(!config->afterStart->commands.contains("SERVICE_DOMAIN"));

    QVERIFY(!config->install->commands.contains("SERVICE_ADDRESS"));
    QVERIFY(!config->start->commands.contains("SERVICE_ADDRESS"));
    QVERIFY(!config->configure->commands.contains("SERVICE_ADDRESS"));
    QVERIFY(!config->afterStart->commands.contains("SERVICE_ADDRESS"));

    delete config;
}


void TestLibrary::testParseDefault() {
    auto *config = new SvdServiceConfig(); /* Load default values */
    QCOMPARE(config->staticPort, -1);
    QVERIFY(config->schedulerActions->length() == 0);
    delete config;
}


void TestLibrary::testMultipleConfigsLoading() {
    auto *config = new SvdServiceConfig(); /* Load default values */
    QVERIFY(config->name == "Default");
    QVERIFY(config->install->commands.length() == 0);
    QVERIFY(config->schedulerActions->length() == 0);
    QVERIFY(config->watchPort == true);
    QVERIFY(config->alwaysOn == true);
    delete config;

    config = new SvdServiceConfig("Redis");
    QCOMPARE(config->name, QString("Redis"));
    QVERIFY(config->install->commands == "sofin get redis");
    QVERIFY(config->watchPort == true);
    QVERIFY(config->alwaysOn == true);
    delete config;

    config = new SvdServiceConfig("Mosh");
    QVERIFY(config->name == "Mosh");
    QVERIFY(config->softwareName == "Mosh");
    QVERIFY(config->install->commands == "sofin get mosh");
    QVERIFY(config->watchPort == false);
    QVERIFY(config->alwaysOn == false);
    delete config;
}


void TestLibrary::testNonExistantConfigLoading() {
    auto *config = new SvdServiceConfig("PlewisŚmiewis");
    QVERIFY(config->name == "PlewisŚmiewis");
    QVERIFY(config->install->commands.length() == 0);
    QVERIFY(config->watchPort == true);
    delete config;
}


void TestLibrary::testFreePortFunctionality() {
    uint port = registerFreeTcpPort(0);
    QVERIFY(port != 0);
    logDebug() << "Port:" << port;

    uint port2 = registerFreeTcpPort();
    uint port3 = registerFreeTcpPort();
    uint port4 = registerFreeTcpPort();
    uint port5 = registerFreeTcpPort();
    uint port6 = registerFreeTcpPort();
    uint port7 = registerFreeTcpPort();
    uint port8 = registerFreeTcpPort();
    uint port9 = registerFreeTcpPort();
    uint port10 = registerFreeTcpPort();
    QVERIFY(port10 != port9 != port8 != port7 != port6 != port5 != port4 != port3 != port2 != port);
    QVERIFY(port2 != 0);
    logDebug() << "Port:" << port2;

    uint takenPort = registerFreeTcpPort(22); // XXX: not yet determined used port.. so using ssh default port
    logDebug() << "Port:" << takenPort;
    QVERIFY(takenPort != 22);
    QVERIFY(takenPort != 0);

    uint takenPort2 = registerFreeTcpPort(1000); // some port under 1024 (root port)
    logDebug() << "Port:" << takenPort2;
    QVERIFY(takenPort2 != 1000);
    QVERIFY(takenPort2 != 0);

    // HACK: XXX: cannot be implemented without working SvdService. It should spawn redis server on given port:
    // uint takenPort3 = registerFreeTcpPort(6379); // some port over 6379 (redis default port)
    // logDebug() << "Port:" << takenPort3;
    // QVERIFY(takenPort3 != 6379);
    // QVERIFY(takenPort3 != 0);
}


void TestLibrary::testJSONParse() {
    const char* fileName = "/tmp/test-file-TestJSONParse.json";
    QString value = "";
    int valueInt = -1;

    writeSampleOf("{\"somekey\": \"somevalue\"}", fileName);
    QFile file(fileName);
    if (!file.exists()) {
        QFAIL("JSON file should exists.");
    }
    file.close();

    auto parsed = parseJSON(fileName);
    value = parsed->get("somekey", "none").asCString();
    QVERIFY(value == QString("somevalue"));

    value = parsed->get("someNOKEY", "none").asCString();
    QVERIFY(value == QString("none"));

    valueInt = parsed->get("someNOKEY", 12345).asInt();
    QVERIFY(valueInt == 12345);

    try {
        valueInt = parsed->get("somekey", 12345).asInt();
        QFAIL("It should throw an exception!");
    } catch (std::exception &e) {
        QCOMPARE(e.what(), "Type is not convertible to int");
    }
    delete parsed;

    file.deleteLater();
}


void TestLibrary::testMemoryAllocations() {
    int amount = 10;
    logDebug() << "Beginning" << amount << "loops of allocation test.";
    for (int i = 0; i < amount; ++i) {
        auto config = new SvdServiceConfig("Redis"); /* Load app specific values */
        usleep(10000); // 1000000 - 1s
        delete config;
    }
}


void TestLibrary::testUtils() {
    uid_t uid = getuid();
    QString homeDir, softwareDataDir, serviceDataDir, name = "Redis";
    #ifdef __linux__
        #define HOME_DIR "/home/"
    #else
        #define HOME_DIR "/Users/"
    #endif

    if (uid == 0)
        homeDir = "/SystemUsers";
    else
        homeDir = QString(HOME_DIR) + getenv("USER");

    softwareDataDir = homeDir + "/SoftwareData";
    serviceDataDir = softwareDataDir + "/" + name;

    QVERIFY(homeDir == getHomeDir());
    QVERIFY(softwareDataDir == getSoftwareDataDir());
    QVERIFY(serviceDataDir == getServiceDataDir(name));
}


void TestLibrary::testSomeRealCraziness() {
    auto *config = new SvdServiceConfig(); /* Load default values */
    config->name = "OmgOmgOmg";
    config->loadIgniter();
    QVERIFY(config->name == "OmgOmgOmg");
    config->name = "";
    config->loadIgniter();
    QVERIFY(config->name == "");
    delete config;
}


void TestLibrary::testSanityValueCheck() {
    auto *config = new SvdServiceConfig("Redis");

    QVERIFY(config->userServiceRoot().contains(getenv("USER")));
    #ifndef __linux__
        QVERIFY(config->userServiceRoot().contains("Users"));
    #endif
    QVERIFY(config->userServiceRoot().contains(QString(DEFAULT_USER_APPS_DIR)));
    QVERIFY(config->userServiceRoot().contains(config->softwareName));

    QVERIFY(config->serviceRoot().contains(QString(SOFTWARE_DIR)));
    QVERIFY(config->serviceRoot().contains(config->softwareName));

    if (config->uid == 0) {
        QVERIFY(config->prefixDir().contains(QString(SYSTEM_USERS_DIR)));
        QVERIFY(!config->prefixDir().contains(QString::number(config->uid))); // root service prefix dir doens't contains uid in path!
    } else {
        QVERIFY(config->prefixDir().contains(QString(USERS_HOME_DIR)));
        QVERIFY(config->prefixDir().contains(getenv("USER")));
    }
    QVERIFY(config->prefixDir().contains(QString(QString(SOFTWARE_DATA_DIR))));
    delete config;
}


/* SvdService tests */
void TestLibrary::testStartingRedis() {
    QString name = "TestRedis";
    auto config = new SvdServiceConfig(name);
    auto service = new SvdService(name);
    service->start();

    service->startSlot(); // should install and start redis
    QString runningFile = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
    QString portsFile = config->prefixDir() + DEFAULT_SERVICE_PORTS_FILE;
    QString domainFile = config->prefixDir() + DEFAULT_SERVICE_DOMAIN_FILE;
    QString pidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
    QString outputFile = config->prefixDir() + DEFAULT_SERVICE_OUTPUT_FILE;
    QVERIFY(QFile::exists(runningFile));

    uint portOfRunningRedis = QString(readFileContents(portsFile).c_str()).trimmed().toUInt();
    logInfo() << "Redis port:" << portOfRunningRedis;
    uint port = registerFreeTcpPort(portOfRunningRedis);
    logInfo() << "Registered port:" << port;
    service->stopSlot();

    QVERIFY(QFile::exists(outputFile));
    QVERIFY(QFile::exists(config->userServiceRoot()));
    QVERIFY(QFile::exists(config->prefixDir()));
    QVERIFY(QFile::exists(portsFile));
    QVERIFY(QFile::exists(domainFile));
    QVERIFY(not QFile::exists(pidFile));
    QVERIFY(not QFile::exists(runningFile));
    QVERIFY(port != portOfRunningRedis);
    // removeDir(config->prefixDir());

    delete service;
    delete config;
}


void TestLibrary::testInstallingWrongRedis() {
    QString name = "TestWrongRedis";
    auto config = new SvdServiceConfig(name);
    auto service = new SvdService(name);
    service->start();
    service->installSlot();
    QString outputFile = config->prefixDir() + DEFAULT_SERVICE_OUTPUT_FILE;
    QString errorsFile = config->prefixDir() + DEFAULT_SERVICE_ERRORS_FILE;
    QVERIFY(expect("some crap", "crap"));
    QVERIFY(not expect("anything", "crap"));
    QVERIFY(QFile::exists(outputFile));
    QVERIFY(QFile::exists(errorsFile));
    // removeDir(config->prefixDir());

    delete service;
    delete config;
}


void TestLibrary::testWebAppDetection() {

    /* static app */
    QString path = testDataDir2;
    if (not QFile::exists(path)) {
        path = testDataDir;
    }
    auto appDetector = new WebAppTypeDetector(path + "/SomeStaticApp");
    logDebug() << "Detected application type:" << appDetector->typeName;
    QVERIFY(appDetector->getType() == StaticSite);
    QVERIFY(appDetector->typeName == "Static");
    delete appDetector;

    appDetector = new WebAppTypeDetector(path + "/SomeRailsApp");
    logDebug() << "Detected application type:" << appDetector->typeName;
    QVERIFY(appDetector->getType() == RailsSite);
    QVERIFY(appDetector->typeName == "Rails");
    delete appDetector;

    appDetector = new WebAppTypeDetector(path + "/SomeNodeApp");
    logDebug() << "Detected application type:" << appDetector->typeName;
    QVERIFY(appDetector->getType() == NodeSite);
    QVERIFY(appDetector->typeName == "Node");
    delete appDetector;

    appDetector = new WebAppTypeDetector(path + "/SomeNoWebApp");
    logDebug() << "Detected application type:" << appDetector->typeName;
    QVERIFY(appDetector->getType() == NoType);
    QVERIFY(appDetector->typeName == "NoType");
    delete appDetector;

}


void TestLibrary::testWebAppDeployer() {

    QString testDomain = "test.my.local.domain";
    QDir().mkdir(getWebAppsDir() + "/" + testDomain);
    touch(getWebAppsDir() + "/" + testDomain + "/package.json");

    auto deployer = new SvdWebAppDeployer(testDomain);
    QVERIFY(deployer->getType() == NodeSite);
    QVERIFY(deployer->getTypeName() == "Node");
    delete deployer;

    testDomain = "test.żółw.pl";
    QDir().mkdir(getWebAppsDir() + "/" + testDomain);
    touch(getWebAppsDir() + "/" + testDomain + "/index.html");

    QVERIFY(QDir().exists(getWebAppsDir() + "/test.żółw.pl"));
    deployer = new SvdWebAppDeployer(testDomain);
    QVERIFY(deployer->getType() == StaticSite);
    QVERIFY(deployer->getTypeName() == "Static");
    delete deployer;

}

