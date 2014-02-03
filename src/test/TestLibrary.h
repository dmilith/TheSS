#ifndef __JSON_TEST_CONFIG__
#define __JSON_TEST_CONFIG__

#include "../jsoncpp/json/json.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/utils.h"
#include "../service_spawner/service.h"
#include "../service_spawner/process.h"
#include "../deployer/webapp_types.h"

#include <QObject>
#include <QtTest/QtTest>


class TestLibrary: public QObject {
    Q_OBJECT
        ConsoleAppender* consoleAppender;
        QString testDataDir, testDataDir2;


        public:
            TestLibrary();
            ~TestLibrary();

        private slots:
            void testParseDefault();
            void testParseJSONRedis();
            void testFreePortFunctionality();
            void testJSONParse();
            void testMemoryAllocations();
            void testMultipleConfigsLoading();
            void testNonExistantConfigLoading();
            void testUtils();
            void testSomeRealCraziness();
            void testSanityValueCheck();
            void testStartingRedis();
            void testInstallingWrongRedis();
            void testWebAppDetection();
            // void testWebAppDeployer();
            void testCrontabEntry();
            void testTail();
            void testMkdir();
            void testUdpPort();
            void testIgniterInjection();
            void testDirRemoval();

};

QTEST_APPLESS_MAIN(TestLibrary)
#include "moc_TestLibrary.cpp"

#endif