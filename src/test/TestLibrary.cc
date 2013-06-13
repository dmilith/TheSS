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
    consoleAppender->setDetailsLevel(Logger::Fatal); /* we don't need logger in test suite by default */
    Logger::registerAppender(consoleAppender);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));

    if (not QDir().exists(DEFAULTSOFTWARETEMPLATESDIR)) {
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

    QVERIFY(config->schedulerActions.first()->cronEntry.contains("*"));
    logDebug() << config->schedulerActions.first()->cronEntry;
    logDebug() << config->schedulerActions.first()->commands;

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
    QVERIFY(config->schedulerActions.length() == 0);
    delete config;
}


void TestLibrary::testMultipleConfigsLoading() {
    auto *config = new SvdServiceConfig(); /* Load default values */
    QVERIFY(config->name == "Default");
    QVERIFY(config->install->commands.length() == 0);
    QVERIFY(config->schedulerActions.length() == 0);
    QVERIFY(config->watchPort == true);
    QVERIFY(config->alwaysOn == true);
    delete config;

    config = new SvdServiceConfig("Redis");
    QCOMPARE(config->name, QString("Redis"));
    QVERIFY(config->afterStop->commands.contains(".running"));
    QVERIFY(config->afterStop->commands.contains("service.pid"));
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

    if (uid == 0)
        homeDir = "/SystemUsers";
    else
        homeDir = getenv("HOME");

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

    QVERIFY(config->userServiceRoot().contains(getenv("HOME")));
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
        QVERIFY(config->prefixDir().contains(getenv("HOME")));
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
    QString domainFile = config->prefixDir() + DEFAULT_SERVICE_DOMAIN_FILE;
    QString pidFile = config->prefixDir() + DEFAULT_SERVICE_PID_FILE;
    QString outputFile = config->prefixDir() + DEFAULT_SERVICE_OUTPUT_FILE;
    QVERIFY(QFile::exists(runningFile));

    QVERIFY(config->afterStop->commands.contains("service.pid"));
    QVERIFY(config->afterStop->commands.contains("kongo bongo"));

    QString portsFile = config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER;
    uint portOfRunningRedis = QString(readFileContents(portsFile).c_str()).trimmed().toUInt();
    logInfo() << "Redis port:" << portOfRunningRedis;
    uint port = registerFreeTcpPort(portOfRunningRedis);
    logInfo() << "Registered port:" << port;
    service->stopSlot();

    QString value = config->replaceAllSpecialsIn(config->validate->commands);
    QVERIFY(not config->validate->commands.contains("SERVICE_PORT"));
    QVERIFY(not config->validate->commands.contains("SERVICE_PORT1"));
    QVERIFY(not config->validate->commands.contains("SERVICE_PORT2"));
    /* read port from file */
    QString valueRead = QString(readFileContents(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/0").c_str()).trimmed();
    QVERIFY(value.contains(valueRead));

    QString valueRead1 = QString(readFileContents(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/1").c_str()).trimmed();
    QVERIFY(value.contains(valueRead1));

    QString valueRead2 = QString(readFileContents(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/2").c_str()).trimmed();
    QVERIFY(value.contains(valueRead2));

    QVERIFY(QDir().exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR));
    QVERIFY(QFile::exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/0"));
    QVERIFY(QFile::exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/1"));
    QVERIFY(QFile::exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/2"));
    // QVERIFY(QFile::exists(outputFile));
    QVERIFY(QFile::exists(config->userServiceRoot()));
    QVERIFY(QFile::exists(config->prefixDir()));
    QVERIFY(QFile::exists(portsFile));
    QVERIFY(QFile::exists(domainFile));
    QVERIFY(not QFile::exists(pidFile));
    QVERIFY(not QFile::exists(runningFile));
    // QVERIFY(port != portOfRunningRedis);
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
    QString errorsFile = config->prefixDir() + ".nothing";
    QVERIFY(expect("some crap", "crap"));
    QVERIFY(not expect("anything", "crap"));
    QVERIFY(QFile::exists(outputFile));
    // QVERIFY(QFile::exists(errorsFile));
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


void TestLibrary::testCrontabEntry() {
    QVERIFY(QString("bug").toInt() == 0);

    /* this is empty case, which is an equivalent of giving entry: "* * * * * ?" */
    auto cron = new SvdCrontab("", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->modes.at(0) == WILDCARD);
    QVERIFY(cron->cronMatch() == true);
    delete cron;

    cron = new SvdCrontab("                     ", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->modes.at(0) == WILDCARD);
    QVERIFY(cron->cronMatch() == true);
    delete cron;

    cron = new SvdCrontab("1 2 3 4 5 ?", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->modes.at(0) == NORMAL);
    QVERIFY(cron->check(1, 0));
    QVERIFY(cron->modes.at(1) == NORMAL);
    QVERIFY(cron->check(2, 1));
    QVERIFY(cron->modes.at(2) == NORMAL);
    QVERIFY(cron->check(3, 2));
    QVERIFY(cron->modes.at(3) == NORMAL);
    QVERIFY(cron->check(4, 3));
    QVERIFY(cron->modes.at(4) == NORMAL);
    QVERIFY(cron->check(5, 4));
    delete cron;

    cron = new SvdCrontab("1,, 2,2,2 ,3 , ,5,5 ?", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1// 2/2/2 /3 / /5/5 ?", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1-- 2-2-2 -3 - -5-5 ?", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("*/10 10-15 32 * 3,4,5,15 ?", "true");
    QVERIFY(cron->modes.at(0) == PERIODIC);
    QVERIFY(cron->check(10, 0));
    QVERIFY(cron->check(40, 0));
    QVERIFY(cron->check(0, 0));
    QVERIFY(not cron->check(11, 0));

    QVERIFY(cron->modes.at(1) == RANGE);
    QVERIFY(cron->check(13, 1)); // 13 is in range of 10-15
    QVERIFY(cron->check(10, 1));
    QVERIFY(cron->check(15, 1));
    QVERIFY(not cron->check(9, 1));
    QVERIFY(not cron->check(16, 1));

    QVERIFY(cron->entries.at(2) == "32");
    QVERIFY(cron->modes.at(2) == NORMAL);

    QVERIFY(cron->modes.at(3) == WILDCARD);

    QVERIFY(cron->check(3, 4));
    QVERIFY(cron->check(4, 4));
    QVERIFY(cron->check(5, 4));
    QVERIFY(cron->check(15, 4));
    QVERIFY(not cron->check(2, 4));
    QVERIFY(not cron->check(6, 4));
    QVERIFY(not cron->check(666, 4));
    QVERIFY(cron->modes.at(4) == SEQUENCE);

    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false); /* it must be false when asking for match for month with 32 days */
    delete cron;

    cron = new SvdCrontab("stefan mariola a b 0/2 * ?", "true");
    QVERIFY(cron->commands == "");
    QVERIFY(cron->cronMatch() == false);
    delete cron;
}


void TestLibrary::testTail() {
    QString a = tail("/usr/include/string.h", 3);
    QVERIFY(a.contains("STRING_H"));
}


void TestLibrary::testMkdir() {
    QVERIFY(QDir(getOrCreateDir("/tmp/abc123")).exists());
    QVERIFY(QDir(getOrCreateDir("/tmp/abc123/4/5/6/7/8/9/zażółcam-gęślą")).exists());
    removeDir("/tmp/abc123/A");
    removeDir("/tmp/abc123/4/5/6/7/8/9/zażółcam-gęślą");
    removeDir("/tmp/abc123");
    QVERIFY(not QDir("/tmp/abc123/A").exists());
    QVERIFY(not QDir("/tmp/abc123/4/5/6/7/8/9/zażółcam-gęślą").exists());
    QVERIFY(not QDir("/tmp/abc123").exists());
}
