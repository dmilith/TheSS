#include <fcntl.h>
#include "TestLibrary.h"

#define TEST_LAG 10


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
    consoleAppender->setDetailsLevel(Logger::Info); /* we don't need logger in test suite by default */
    Logger::registerAppender(consoleAppender);
    setDefaultEncoding();

    // if (not QDir().exists(DEFAULT_SOFTWARE_TEMPLATES_DIR)) {
    //     cout << "Install igniters first before launching test!" << endl;
    //     exit(1);
    // }
    testDataDir = QDir::currentPath() + "/basesystem/universal/TestData";
    testDataDir2 = QDir::currentPath() + "/../basesystem/universal/TestData";
}

/* test functions */

void TestLibrary::testDirRemoval() {
    QDir().mkdir("/tmp/abc0123");
    QDir().mkdir("/tmp/abc0123/123");
    QDir().mkdir("/tmp/abc0123/456");
    touch("/tmp/abc0123/456/zouza");
    touch("/tmp/abc0123/456/zouza123");
    touch("/tmp/abc0123/456/badziąg");
    QDir().mkdir("/tmp/abc0123/456/ciapag/zdamly");
    touch("/tmp/abc0123/456/ciapag/zdamly/123");
    touch("/tmp/abc0123/456/ciapag/zdamly/456");
    touch("/tmp/abc0123/456/ciapag/zdamly/abc");
    QVERIFY(removeDir("/tmp/abc0123"));
    QVERIFY(removeDir("/tmp/I_dont_exist") == false);
}


// void TestLibrary::testParseJSONRedis() {
//     auto config = new SvdServiceConfig("Redis"); /* Load app specific values */
//     QCOMPARE(config->name, QString("Redis"));
//     QCOMPARE(config->softwareName, QString("Redis"));
//     QCOMPARE(config->staticPort, -1);
//     QCOMPARE(config->uid, getuid());
//     QVERIFY(config->schedulers.first()->cronEntry.contains("*"));
//     // logDebug() << config->schedulers.first()->cronEntry;
//     // logDebug() << config->schedulers.first()->commands;

//     /* verify replaceAllIn result, should not contain SERVICE_PORT, SERVICE_DOMAIN, SERVICE_ROOT, SERVICE_ADDRESS */
//     // QVERIFY(!config->install->commands.contains("SERVICE_PORT"));
//     // QVERIFY(!config->start->commands.contains("SERVICE_PORT"));
//     // QVERIFY(!config->configure->commands.contains("SERVICE_PORT"));
//     // QVERIFY(!config->afterStart->commands.contains("SERVICE_PORT"));

//     QVERIFY(!config->install->commands.contains("SERVICE_ROOT"));
//     QVERIFY(!config->start->commands.contains("SERVICE_ROOT"));
//     QVERIFY(!config->configure->commands.contains("SERVICE_ROOT"));
//     QVERIFY(!config->afterStart->commands.contains("SERVICE_ROOT"));

//     QVERIFY(!config->install->commands.contains("SERVICE_DOMAIN"));
//     QVERIFY(!config->start->commands.contains("SERVICE_DOMAIN"));
//     QVERIFY(!config->configure->commands.contains("SERVICE_DOMAIN"));
//     QVERIFY(!config->afterStart->commands.contains("SERVICE_DOMAIN"));

//     QVERIFY(!config->install->commands.contains("SERVICE_ADDRESS"));
//     QVERIFY(!config->start->commands.contains("SERVICE_ADDRESS"));
//     QVERIFY(!config->configure->commands.contains("SERVICE_ADDRESS"));
//     QVERIFY(!config->afterStart->commands.contains("SERVICE_ADDRESS"));

//     delete config;
// }


void TestLibrary::testLoadingDefault() {
    auto config = new SvdServiceConfig();
    QStringList input;
    char *path[0];
    int i = 0;
    foreach (QString s, input) {
        path[i] = new char[s.length()];
        strncpy(path[i], s.toUtf8().constData(), s.length() + 1);
        path[s.length()] = DEFAULT_ZERO_CHAR;
        i++;
    }
    path[i] = DEFAULT_ZERO_CHAR;
    QVERIFY(config->getDouble("formatVersion") != 0);
    QVERIFY(config->getDouble("formatVersion") > 0.88);
    QVERIFY(config->getBoolean("alwaysOn") == true);
    QVERIFY(config->getBoolean("stefan") == false);
    QVERIFY(not config->shell.isEmpty());
    QVERIFY(config->valid());
    delete config;
}


void TestLibrary::testParseDefault() {
    QString testParseDefault = "ab/cd";
    QVERIFY(testParseDefault.split("2").size() == 1);
    QVERIFY(testParseDefault.split("/").size() == 2);
    char errbuf[1024];
    auto config = new SvdServiceConfig(); /* Load default values */
    QVERIFY(config->standaloneDependencies.size() > 0);
    QVERIFY(config->valid());

    /* parse arrays test */
    const char* testParse2 = "{\"stefan\": [\"fst\", \"scnd\"], \"some\": {\"internal\": [\"a\",\"b\",\"c\"]}}";
    auto node = yajl_tree_parse(testParse2, errbuf, sizeof(errbuf));
    QVERIFY(node != NULL);
    for( int i = 0; i < 100; i++) {
        QVERIFY(JSONAPI::getArray(node, NULL, "stefan").first() == "fst");
        QVERIFY(JSONAPI::getArray(node, NULL, "stefan").last() == "scnd");
        QVERIFY(JSONAPI::getArray(node, NULL, "stefan").size() == 2);
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").contains("a"));
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").contains("b"));
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").contains("c"));
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").size() == 3);
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    /* parse strings, plus additional hierarchy test */
    const char* testParse = "{\"stoo\":\"111\", \"abc\": \"oO\", \"ddd\": {\"some\": true, \"zabra\": \"888\", \"abra\": \"666\", \"zada\": {\"abra\": \"777\"}}}";
    node = yajl_tree_parse(testParse, errbuf, sizeof(errbuf));
    logWarn() << errbuf;
    QVERIFY(node != NULL);
    for( int i = 0; i < 5; i++) {
        QVERIFY(JSONAPI::getString(node, NULL, "abc") == "oO");
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/some") == true);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "nothere") == false);
        QVERIFY(JSONAPI::getString(node, NULL, "ddd/zabra") == "888");
        QVERIFY(JSONAPI::getString(node, NULL, "stoo") == "111");
        QVERIFY(JSONAPI::getString(node, NULL, "ddd/abra") == "666");
        QVERIFY(JSONAPI::getString(node, NULL, "ddd/zada/abra") == "777");
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    /* parse bools, plus additional hierarchy test */
    testParse = "{\"stoo\":true, \"abc\": false, \"ddd\": {\"some\": false, \"zabra\": true, \"abra\": true, \"zada\": {\"abra\": true}}}";
    node = yajl_tree_parse(testParse, errbuf, sizeof(errbuf));
    logWarn() << errbuf;
    QVERIFY(node != NULL);
    for( int i = 0; i < 100; i++) {
        QVERIFY(JSONAPI::getBoolean(node, NULL, "abc") == false);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/some") == false);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/abra") == true);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/zada/abra") == true);
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    /* parse numbers, plus additional hierarchy test */
    testParse = "{\"stoo\":111, \"abc\": -1, \"ddd\": {\"some\": 1, \"zabra\": 2, \"abra\": 3, \"zada\": {\"abra\": 4}}}";
    node = yajl_tree_parse(testParse, errbuf, sizeof(errbuf));
    logWarn() << errbuf;
    QVERIFY(node != NULL);
    QVERIFY(JSONAPI::getInteger(node, NULL, "abc") == -1);
    QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/some") == 1);
    QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/abra") == 3);
    QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/zada/abra") == 4);

    for (int o = 0; o < 100; o++) {
        QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/zada/abra") == 4);
        usleep(100 * TEST_LAG);
    }

    for (int o = 0; o < 100; o++) {
        QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/abra") == 3);
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    delete config;
}


void TestLibrary::testConfigDryRun() {
    auto config = new SvdServiceConfig("TestRedis", true); /* dry run */
    QVERIFY(config->install->commands == "sofin get redis");
    QVERIFY(config->softwareName == "Redis");
    QVERIFY(not config->shell.isEmpty());
    QVERIFY(config->shell == "/bin/bash");
    QVERIFY(not config->domains.contains("localhost"));
    QVERIFY(config->domains.contains("ene"));
    QVERIFY(config->domains.contains("due"));
    QVERIFY(config->domains.contains("rabe"));
    QVERIFY(config->afterStart->expectOutput == config->afterStart->commands);
    QVERIFY(config->afterStart->commands.contains("ene due rabe"));
    QVERIFY(not QDir().exists(getServiceDataDir(config->name)));
    QVERIFY(config->valid());
    delete config;
}


void TestLibrary::testParseExistingIgniter() {
    QString testParseDefault = "ab/cd";
    QVERIFY(testParseDefault.split("2").size() == 1);
    QVERIFY(testParseDefault.split("/").size() == 2);
    char errbuf[1024];
    auto config = new SvdServiceConfig("Redis"); /* Load Redis igniter values */
    QVERIFY(config->standaloneDependencies.size() > 0);
    QVERIFY(config->standaloneDependencies.contains("redis-usock"));
    QVERIFY(config->standaloneDependencies.contains("mysql"));
    QVERIFY(config->valid());

    /* parse arrays test */
    const char* testParse2 = "{\"stefan\": [\"fst\", \"scnd\"], \"some\": {\"internal\": [\"a\",\"b\",\"c\"]}}";
    auto node = yajl_tree_parse(testParse2, errbuf, sizeof(errbuf));
    QVERIFY(node != NULL);
    for( int i = 0; i < 100; i++) {
        QVERIFY(JSONAPI::getArray(node, NULL, "stefan").first() == "fst");
        QVERIFY(JSONAPI::getArray(node, NULL, "stefan").last() == "scnd");
        QVERIFY(JSONAPI::getArray(node, NULL, "stefan").size() == 2);
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").contains("a"));
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").contains("b"));
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").contains("c"));
        QVERIFY(JSONAPI::getArray(node, NULL, "some/internal").size() == 3);
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    /* parse strings, plus additional hierarchy test */
    const char* testParse = "{\"stoo\":\"111\", \"abc\": \"oO\", \"ddd\": {\"some\": true, \"zabra\": \"888\", \"abra\": \"666\", \"zada\": {\"abra\": \"777\"}}}";
    node = yajl_tree_parse(testParse, errbuf, sizeof(errbuf));
    logWarn() << errbuf;
    QVERIFY(node != NULL);
    for( int i = 0; i < 5; i++) {
        QVERIFY(JSONAPI::getString(node, NULL, "abc") == "oO");
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/some") == true);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "nothere") == false);
        QVERIFY(JSONAPI::getString(node, NULL, "ddd/zabra") == "888");
        QVERIFY(JSONAPI::getString(node, NULL, "stoo") == "111");
        QVERIFY(JSONAPI::getString(node, NULL, "ddd/abra") == "666");
        QVERIFY(JSONAPI::getString(node, NULL, "ddd/zada/abra") == "777");
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    /* parse bools, plus additional hierarchy test */
    testParse = "{\"stoo\":true, \"abc\": false, \"ddd\": {\"some\": false, \"zabra\": true, \"abra\": true, \"zada\": {\"abra\": true}}}";
    node = yajl_tree_parse(testParse, errbuf, sizeof(errbuf));
    logWarn() << errbuf;
    QVERIFY(node != NULL);
    for( int i = 0; i < 100; i++) {
        QVERIFY(JSONAPI::getBoolean(node, NULL, "abc") == false);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/some") == false);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/abra") == true);
        QVERIFY(JSONAPI::getBoolean(node, NULL, "ddd/zada/abra") == true);
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    /* parse numbers, plus additional hierarchy test */
    testParse = "{\"stoo\":111, \"abc\": -1, \"ddd\": {\"some\": 1, \"zabra\": 2, \"abra\": 3, \"zada\": {\"abra\": 4}}}";
    node = yajl_tree_parse(testParse, errbuf, sizeof(errbuf));
    logWarn() << errbuf;
    QVERIFY(node != NULL);
    QVERIFY(JSONAPI::getInteger(node, NULL, "abc") == -1);
    QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/some") == 1);
    QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/abra") == 3);
    QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/zada/abra") == 4);

    for (int o = 0; o < 100; o++) {
        QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/zada/abra") == 4);
        usleep(100 * TEST_LAG);
    }

    for (int o = 0; o < 100; o++) {
        QVERIFY(JSONAPI::getInteger(node, NULL, "ddd/abra") == 3);
        usleep(100 * TEST_LAG);
    }
    yajl_tree_free(node);

    delete config;
}


void TestLibrary::testJsonValidityOfIgniters() {
    QStringList igniters;
    if (getuid() == 0) {
        igniters << "Pptpd" << "Openvpn" << "Ntp" << "LiveUsers" << "Courier" << "Coreginx" << "Bind" << "Bind-WithZone";
    }
    igniters << "Memcached" << "Php" << "Redis" << "Redis-usock" << "Postgresql" << "Passenger" << "Passenger19" << "ProcessDataCollector" << "Mysql" << "Mysql-master" << "Mysql-usock" << "Mysql51" << "Mysql-slave" << "Mosh" << "Mongodb" << "Mongodb-slave" << "Mongodb-master" << "Dropbear" << "Elasticsearch";
    Q_FOREACH(QString igniter, igniters) {
        auto config = new SvdServiceConfig(igniter);
        QVERIFY(not config->softwareName.isEmpty());
        QVERIFY(config->valid());
        delete config;
    }
}


void TestLibrary::testMultipleConfigsLoading() {
    // auto *config = new SvdServiceConfig(); /* Load default values */
    // QVERIFY(config->name == "Default");
    // QVERIFY(config->install->commands.isEmpty());
    // QVERIFY(config->schedulers.length() == 0);
    // QVERIFY(config->watchPort == true);
    // QVERIFY(config->alwaysOn == true);
    // QVERIFY(config->resolveDomain == false);
    // delete config;

    auto config = new SvdServiceConfig("Redis");
    QCOMPARE(config->name, QString("Redis"));
    QVERIFY(config->valid());
    // QVERIFY(config->afterStop->commands.contains(".running"));
    // QVERIFY(config->afterStop->commands.contains("service.pid"));
    QVERIFY(config->install->commands == "sofin get redis");
    QVERIFY(config->watchPort == true);
    QVERIFY(config->alwaysOn == true);
    QVERIFY(config->resolveDomain == false);
    delete config;

    config = new SvdServiceConfig("Mosh");
    QVERIFY(config->name == "Mosh");
    QVERIFY(config->valid());
    QVERIFY(config->softwareName == "Mosh");
    QVERIFY(config->install->commands == "sofin get mosh");
    QVERIFY(config->watchPort == false);
    QVERIFY(config->alwaysOn == false);
    QVERIFY(config->resolveDomain == false);
    delete config;
}


void TestLibrary::testNonExistantConfigLoading() {
    auto *config = new SvdServiceConfig("PlewisŚmiewis");
    QVERIFY(config->errors().contains("premature EOF"));
    QVERIFY(config->name == "PlewisŚmiewis");
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

    if (getuid() > 0) {
        uint takenPort2 = registerFreeTcpPort(1000); // some port under 1024 (root port)
        logDebug() << "Port:" << takenPort2;
        QVERIFY(takenPort2 != 1000);
        QVERIFY(takenPort2 != 0);
    }
    // HACK: XXX: cannot be implemented without working SvdService. It should spawn redis server on given port:
    // uint takenPort3 = registerFreeTcpPort(6379); // some port over 6379 (redis default port)
    // logDebug() << "Port:" << takenPort3;
    // QVERIFY(takenPort3 != 6379);
    // QVERIFY(takenPort3 != 0);
}


// void TestLibrary::testJSONParse() {
//     const char* fileName = "/tmp/test-file-TestJSONParse.json";
//     QString value = "";
//     int valueInt = -1;

//     writeSampleOf("{\"somekey\": \"somevalue\"}", fileName);
//     QFile file(fileName);
//     if (!file.exists()) {
//         QFAIL("JSON file should exists.");
//     }
//     file.close();

//     auto parsed = parseJSON(fileName);
//     value = parsed->get("somekey", "none").asCString();
//     QVERIFY(value == QString("somevalue"));

//     value = parsed->get("someNOKEY", "none").asCString();
//     QVERIFY(value == QString("none"));

//     valueInt = parsed->get("someNOKEY", 12345).asInt();
//     QVERIFY(valueInt == 12345);

//     try {
//         valueInt = parsed->get("somekey", 12345).asInt();
//         QFAIL("It should throw an exception!");
//     } catch (std::exception &e) {
//         QCOMPARE(e.what(), "Type is not convertible to int");
//     }
//     delete parsed;

//     file.deleteLater();
// }


void TestLibrary::testMemoryAllocations() {
    int amount = 10;
    logDebug() << "Beginning" << amount << "loops of allocation test.";
    for (int i = 0; i < amount; ++i) {
        SvdServiceConfig *config = new SvdServiceConfig("Redis"); /* Load app specific values */
        QVERIFY(config->valid());
        usleep(10000); // 1000000 - 1s
        delete config;
    }
}


void TestLibrary::testUtils() {
    uid_t uid = getuid();
    QString homeDir, softwareDataDir, serviceDataDir, name = "Redis";

    if (uid == 0)
        homeDir = "/SystemUsers/";
    else
        homeDir = getenv("HOME");

    softwareDataDir = homeDir + "/SoftwareData";
    serviceDataDir = softwareDataDir + "/" + name;

    QVERIFY(homeDir == getHomeDir());
    QVERIFY(softwareDataDir != getSoftwareDataDir(uid));
    // QVERIFY(serviceDataDir == getServiceDataDir(name));
}


void TestLibrary::testSomeRealCraziness() {
    auto *config = new SvdServiceConfig("OmgOmgOmg"); /* Load default values */
    config->loadIgniter();
    QVERIFY(config->errors().contains("premature EOF"));
    QVERIFY(config->name == "OmgOmgOmg");
    QVERIFY(config->valid() == false);
    config->name = "";
    config->loadIgniter();
    QVERIFY(config->name == "");
    delete config;
}


void TestLibrary::testSanityValueCheck() {
    auto *config = new SvdServiceConfig("Redis");
    QVERIFY(config->valid());
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
// void TestLibrary::testStartingRedis() {
//     QString name = "TestRedis";
//     auto config = new SvdServiceConfig(name);
//     auto service = new SvdService(name);
//     service->start();

//     service->startSlot(); // should install and start redis
//     QString runningFile = config->prefixDir() + DEFAULT_SERVICE_RUNNING_FILE;
//     // QString domainFile = config->prefixDir() + DEFAULT_SERVICE_DOMAIN_FILE;
//     QString pidFile = config->prefixDir() + config->releaseName() + DEFAULT_SERVICE_PIDS_DIR +  DEFAULT_SERVICE_PID_FILE;
//     QString outputFile = config->prefixDir() + DEFAULT_SERVICE_OUTPUT_FILE;
//     QVERIFY(QFile::exists(runningFile));

//     // QVERIFY(config->afterStop->commands.contains("service.pid"));
//     QVERIFY(config->afterStop->commands.contains("kongo bongo"));

//     QString portsFile = config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER;
//     uint portOfRunningRedis = readFileContents(portsFile).trimmed().toUInt();
//     logInfo() << "Redis port:" << portOfRunningRedis;
//     uint port = registerFreeTcpPort(portOfRunningRedis);
//     logInfo() << "Registered port:" << port;
//     service->stopSlot();

//     QString value = config->replaceAllSpecialsIn(config->validate->commands);
//     // QVERIFY(not config->validate->commands.contains("SERVICE_PORT"));
//     // QVERIFY(not config->validate->commands.contains("SERVICE_PORT1"));
//     // QVERIFY(not config->validate->commands.contains("SERVICE_PORT2"));
//     /* read port from file */
//     // QString valueRead = QString(readFileContents(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/0").c_str()).trimmed();
//     // QVERIFY(value.contains(valueRead));

//     // QString valueRead1 = QString(readFileContents(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/1").c_str()).trimmed();
//     // QVERIFY(value.contains(valueRead1));

//     // QString valueRead2 = QString(readFileContents(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/2").c_str()).trimmed();
//     // QVERIFY(value.contains(valueRead2));

//     QVERIFY(QDir().exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR));
//     // QVERIFY(QFile::exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/0"));
//     // QVERIFY(QFile::exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/1"));
//     // QVERIFY(QFile::exists(config->prefixDir() + DEFAULT_SERVICE_PORTS_DIR + "/2"));
//     // QVERIFY(QFile::exists(outputFile));
//     if (getuid > 0)
//         QVERIFY(QFile::exists(config->userServiceRoot()));
//     QVERIFY(QFile::exists(config->prefixDir()));
//     // QVERIFY(QFile::exists(portsFile));
//     // QVERIFY(QFile::exists(domainFile));
//     QVERIFY(not QFile::exists(pidFile));
//     QVERIFY(not QFile::exists(runningFile));
//     // QVERIFY(port != portOfRunningRedis);
//     // removeDir(config->prefixDir());

//     delete service;
//     delete config;
// }


void TestLibrary::testInstallingWrongRedis() {
    QString name = "TestWrongRedis";
    auto config = new SvdServiceConfig(name);
    QVERIFY(config->valid());
    auto service = new SvdService(name);
    service->start();
    service->installSlot();
    QString outputFile = config->prefixDir() + DEFAULT_SERVICE_OUTPUT_FILE;
    QString errorsFile = config->prefixDir() + ".nothing";
    QVERIFY(expect("some crap", "crap"));
    QVERIFY(not expect("anything", "crap"));
    // QVERIFY(QFile::exists(outputFile));
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
    QVERIFY(appDetector->getType() == RubySite);
    QVERIFY(appDetector->typeName == "Ruby");
    delete appDetector;

    appDetector = new WebAppTypeDetector(path + "/SomeNodeApp");
    logDebug() << "Detected application type:" << appDetector->typeName;
    QVERIFY(appDetector->getType() == NodeSite);
    QVERIFY(appDetector->typeName == "Node");
    delete appDetector;

    appDetector = new WebAppTypeDetector(path + "/SomeNoWebApp");
    logDebug() << "Detected application type:" << appDetector->typeName;
    QVERIFY(appDetector->getType() == StaticSite);
    QVERIFY(appDetector->typeName == "Static");
    delete appDetector;

}


// void TestLibrary::testWebAppDeployer() {

//     QString testDomain = "test.my.local.domain";
//     QDir().mkdir(getWebAppsDir() + "/" + testDomain);
//     touch(getWebAppsDir() + "/" + testDomain + "/package.json");

//     auto deployer = new SvdWebAppDeployer(testDomain);
//     QVERIFY(deployer->getType() == NodeSite);
//     QVERIFY(deployer->getTypeName() == "Node");
//     delete deployer;

//     testDomain = "test.żółw.pl";
//     QDir().mkdir(getWebAppsDir() + "/" + testDomain);
//     touch(getWebAppsDir() + "/" + testDomain + "/index.html");

//     QVERIFY(QDir().exists(getWebAppsDir() + "/test.żółw.pl"));
//     deployer = new SvdWebAppDeployer(testDomain);
//     QVERIFY(deployer->getType() == StaticSite);
//     QVERIFY(deployer->getTypeName() == "Static");
//     delete deployer;

// }


void TestLibrary::testCrontabEntry() {
    QVERIFY(QString("bug").toInt() == 0);

    /* this is empty case, which is an equivalent of giving entry: "* * * * * ?!" */
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

    cron = new SvdCrontab("1 2 3 4 5 ?!", "true");
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

    cron = new SvdCrontab("1,, 2,2,2 ,3 , ,5,5 ?!", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1// 2/2/2 /3 / /5/5 ?!", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1-- 2-2-2 -3 - -5-5 ?!", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("*/10 10-15 32 * 3,4,5,15 ?!", "true");
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

    cron = new SvdCrontab("stefan mariola a b 0/2 * ?!", "true");
    QVERIFY(cron->commands == "true");
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1 2 3 0/2 * ?", "true");
    QVERIFY(cron->commands.isEmpty());
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1 2 3 0/2 * !", "true");
    QVERIFY(cron->commands.isEmpty());
    QVERIFY(cron->cronMatch() == false);
    delete cron;

    cron = new SvdCrontab("1 2 3 0/2 *", "true");
    QVERIFY(cron->commands.isEmpty());
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


void TestLibrary::testUdpPort() {
    // int port = registerFreeUdpPort(12345);
    // QVERIFY(port != 12345);
    int port = registerFreeUdpPort(12346);
    QVERIFY(port == 12346);
}


void TestLibrary::testIgniterInjection() {
    /*
    SERVICE_INSTALL_HOOK
    SERVICE_START_HOOK
    SERVICE_STOP_HOOK
    SERVICE_AFTERSTART_HOOK
    SERVICE_AFTERSTOP_HOOK
    SERVICE_CONFIGURE_HOOK
    SERVICE_BABYSITTER_HOOK
    SERVICE_VALIDATE_HOOK
    */
    // auto config2 = new SvdServiceConfig();
    // delete config2;
    QString name = "TestRedisInjected";
    auto config = new SvdServiceConfig(name);
    QVERIFY(not config->configure->commands.contains("SERVICE_START_HOOK"));
    QVERIFY(not config->configure->commands.contains("SERVICE_AFTERSTART_HOOK"));
    QVERIFY(not config->configure->commands.contains("SERVICE_STOP_HOOK"));
    QVERIFY(not config->install->commands.contains("SERVICE_AFTERSTOP_HOOK"));
    QVERIFY(config->softwareName == "Redis");
    QVERIFY(config->portsPool == 11);
    QVERIFY(not config->alwaysOn);
    QVERIFY(not config->resolveDomain);
    QVERIFY(config->watchPort);
    QVERIFY(not config->watchUdpPort);
    QVERIFY(not config->watchSocket);
    QVERIFY(config->watchHttpAddresses.length() == 0);
    QVERIFY(not config->autoStart);
    QVERIFY(not config->webApp);
    QVERIFY(config->staticPort == -1);
    QVERIFY(config->startOrder == 10);
    QVERIFY(config->configureOrder == config->startOrder);
    QVERIFY(config->afterStop->commands.contains("kongo bongo"));
    QVERIFY(config->install->commands.contains("kongo bongo"));
    delete config;
}
