/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include "../globals/globals.h"
#include "../jsoncpp/json/json.h"
#include "../cutelogger/Logger.h"
#include "../cutelogger/ConsoleAppender.h"
#include "../cutelogger/FileAppender.h"
#include "../service_spawner/process.h"

#define logTrace LOG_TRACE
#define logDebug LOG_DEBUG
#define logInfo  LOG_INFO
#define logWarn  LOG_WARNING
#define logError LOG_ERROR
#define logFatal LOG_FATAL

#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#include <QtCore>
#include <QTime>
#include <QTextCodec>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QNetworkInterface>


using namespace std;


QList<int> gatherUserUids();

string readFileContents(const QString& fileName);

bool expect(const QString& inputFileContent, const QString& expectedString);
bool removeDir(const QString& dirName);
// bool cpDir(const QString &srcPath, const QString &dstPath);
bool setPublicDirPriviledges(const QString& path);
bool setUserDirPriviledges(const QString& path);
bool pidIsAlive(uint pid);

void shutdownDefaultVPNNetwork();
void setupDefaultVPNNetwork();
void performCleanupOfOldLogs();
void touch(const QString& fileName);
void writeToFile(const QString& fileName, const QString& contents);
void writeToFile(const QString& fileName, const QString& contents, bool rotateFile);
void rotateFile(const QString& fileName);
void unixSignalHandler(int sigNum);

uint registerFreeTcpPort(uint specificPort = 0);
Json::Value* parseJSON(const QString& filename);

const QString toHMS(uint duration);
const QString getOrCreateDir(const QString& path);
const QString getWebAppsDir();
const QString getHomeDir();
const QString getSoftwareDataDir();
const QString getServiceDataDir(const QString& name);
const QString getHomeDir(uid_t uid);
const QString getSoftwareDataDir(uid_t uid);
const QString getServiceDataDir(uid_t uid, const QString& name);

#endif
