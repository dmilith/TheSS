/**
 *  @author tallica, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "utils.h"



void setupDefaultVPNNetwork() {
    #ifdef __FreeBSD__
        logInfo() << "Launching VPN Network Setup";
        auto proc = new SvdProcess("VPN-setup", 0, false); // don't redirect output
        proc->spawnProcess(DEFAULT_VPN_INTERFACE_SETUP_COMMAND);
        proc->waitForFinished(-1);
        proc->close();
        delete proc;
    #endif
}


void shutdownDefaultVPNNetwork() {
    #ifdef __FreeBSD__
        logInfo() << "Shutting down VPN Network Setup";
        auto proc = new SvdProcess("VPN-setup", 0, false); // don't redirect output
        proc->spawnProcess(DEFAULT_VPN_INTERFACE_SHUTDOWN_COMMAND);
        proc->waitForFinished(-1);
        proc->close();
        delete proc;
    #endif
}


QList<int> gatherUserUids() {
    auto userDirs = QDir(USERS_HOME_DIR).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QList<int> dirs;

    /* filter through invalid directories */
    Q_FOREACH(QString directory, userDirs) {
        bool ok;
        int validUserDir = directory.toInt(&ok, 10); /* valid user directory must be number here */
        if (ok)
            dirs << validUserDir;
        else
            logTrace() << "Filtering out userDir:" << directory;
    }
    return dirs;
}


void rotateFile(const QString& fileName) {
    if (QFile::exists(fileName)) {
        logDebug() << "Rotating file:" << fileName;
        const QDateTime now = QDateTime::currentDateTime();
        const QString fullPath = QFileInfo(fileName).absoluteFilePath();
        const QStringList splt = fullPath.split("/");
        const QString fileStandaloneName = splt.value(splt.length() - 1);
        const QString parentCwdFolderName = splt.value(splt.length() - 2);
        const QString destLogsDir = getHomeDir() + LOGS_DIR + "/" + parentCwdFolderName;
        const QString destinationFile = destLogsDir + "/" + fileStandaloneName + "." + now.toString(".yyyy-MM-dd--hh_mm_ss");

        logTrace() << "Log folder name appendix:" << parentCwdFolderName;
        logDebug() << "Rotate file:" << fileName << ", Logs dir:" << destLogsDir;
        getOrCreateDir(destLogsDir);

        logTrace() << "Destination file:" << destinationFile;
        QFile::copy(fileName, destinationFile);
        performCleanupOfOldLogs();

    } else {
        logDebug() << "No file found to rotate:" << fileName;
    }
}


void performCleanupOfOldLogs() {
    auto rootDirectory = getOrCreateDir(getHomeDir() + LOGS_DIR);
    auto logDirs = QDir(rootDirectory).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    logDebug() << "Log dirs cleanup invoke in dir with:" << logDirs.length() << "elements, Root Logs DIR:" << rootDirectory;
    Q_FOREACH(QString directory, logDirs) {
        auto cwdFiles = QDir(rootDirectory + "/" + directory).entryList(QDir::Files | QDir::Hidden, QDir::Name);
        auto fileAmount = cwdFiles.length();
        if (fileAmount > AMOUNT_OF_LOG_FILES_TO_KEEP) {
            for (int i = fileAmount; i > AMOUNT_OF_LOG_FILES_TO_KEEP; i--) {
                QString file = rootDirectory + "/" + directory + "/" + cwdFiles.at(fileAmount - i);
                logTrace() << "Removing old log file:" << file;
                QFile::remove(file);
            }
        }

        logTrace() << "Cleanup of dir:" << rootDirectory + "/" + directory << ", with files:" << cwdFiles.length();
    }
}


// bool cpDir(const QString &srcPath, const QString &dstPath) {
//     // removeDir(dstPath);
//     QDir parentDstDir(QFileInfo(dstPath).path());
//     if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
//         return false;

//     QDir srcDir(srcPath);
//     foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
//         QString srcItemPath = srcPath + "/" + info.fileName();
//         QString dstItemPath = dstPath + "/" + info.fileName();
//         if (info.isDir()) {
//             if (!cpDir(srcItemPath, dstItemPath)) {
//                 return false;
//             }
//         } else if (info.isFile()) {
//             if (!QFile::copy(srcItemPath, dstItemPath)) {
//                 return false;
//             }
//         } else {
//             logDebug() << "Unhandled item" << info.filePath() << "in cpDir";
//         }
//     }
//     return true;
// }


bool pidIsAlive(uint pid) {
    int result = kill(pid, 0);
    if (result == 0)
        return true;
    else
        return false;
}


void unixSignalHandler(int sigNum) {
    if (sigNum == SIGINT) {
        logWarn() << "Caught SIGINT signal. Quitting application.";
        qApp->quit();
    }
}


bool setPublicDirPriviledges(const QString& path) {
    char mode[] = "0733"; // no read: rwx-wx-wx equivalent
    int i = strtol(mode, 0, 8);
    auto err = errno;
    if (chmod(path.toUtf8(), i) < 0) {
        logDebug() << "Error chmoding:" << strerror(err);
        return false;
    }
    return true;
}


bool setUserDirPriviledges(const QString& path) {
    char mode[] = "0711"; // no read: rwx--x--x equivalent
    int i = strtol(mode, 0, 8);
    auto err = errno;
    if (chmod(path.toUtf8(), i) < 0) {
        logDebug() << "Error chmoding:" << strerror(err);
        return false;
    }
    return true;
}


bool expect(const QString& inputFileContent, const QString& expectedString) {
    return inputFileContent.trimmed().contains(expectedString.trimmed());
}


const QString getOrCreateDir(const QString& path) {
    if (not QFile::exists(path)) {
        logTrace() << "Creating non existant dir:" << path ;
        QDir().mkdir(path);
    }
    return path;
}


const QString toHMS(uint duration) {
    QString res;
    const int seconds = (int)(duration % 60);
    duration /= 60;
    const int minutes = (int)(duration % 60);
    duration /= 60;
    const int hours = (int)(duration % 24);
    const int days = (int)(duration / 24);

    if ((hours == 0) && (days == 0))
        return res.sprintf("%02d:%02d", minutes, seconds);
    if (days == 0)
        return res.sprintf("%02d:%02d:%02d", hours, minutes, seconds);
    return res.sprintf("%dd%02d:%02d:%02d", days, hours, minutes, seconds);
}


void touch(const QString& fileName) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.close();
    return;
}


bool removeDir(const QString& dirName) {
    bool result = false;
    QDir dir(dirName);
    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }
            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}


void writeToFile(const QString& fileName, const QString& contents) {
    writeToFile(fileName, contents, true);
}


void writeToFile(const QString& fileName, const QString& contents, bool rotateThisFile) {
    if (rotateThisFile)
        rotateFile(fileName);
    QFile file(fileName);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << contents << endl;
    }
    file.close();
}


const QString getWebAppsDir() {
    return getOrCreateDir(getHomeDir() + DEFAULT_WEBAPPS_DIR);
}


const QString getHomeDir(uid_t uid) {
    if (uid == 0)
        return QString(SYSTEM_USERS_DIR);
    else
        return QString(USERS_HOME_DIR) + "/" + QString::number(uid);
}


const QString getSoftwareDataDir(uid_t uid) {
    QString dataDir = getHomeDir(uid) + QString(SOFTWARE_DATA_DIR);
    if (!QFile::exists(dataDir)) {
        logTrace() << "Software data dir:" << dataDir << ", doesn't exists. Creating it.";
        QDir().mkpath(dataDir);
    }
    return dataDir;
}


const QString getServiceDataDir(uid_t uid, const QString& name) {
    return getSoftwareDataDir(uid) + "/" + name;
}


const QString getHomeDir() {
    return getHomeDir(getuid());
}


const QString getSoftwareDataDir() {
    return getSoftwareDataDir(getuid());
}


const QString getServiceDataDir(const QString& name) {
    return getServiceDataDir(getuid(), name);
}


/* author: dmilith */
uint registerFreeTcpPort(uint specificPort) {
    QTime midnight(0, 0, 0);
    qsrand(midnight.msecsTo(QTime::currentTime())); // accuracy is in ms.. so let's hack it a bit
    usleep(10000); // this practically means no chance to generate same port when generating multiple ports at once
    uint port = 0, rand = (qrand() % 40000);
    if (specificPort == 0) {
        port = 10000 + rand;
    } else
        port = specificPort;

    logTrace() << "Trying port: " << port << ". Randseed: " << rand;
    auto inter = new QNetworkInterface();
    auto list = inter->allAddresses(); /* all addresses on all interfaces */
    logDebug() << "Addresses amount: " << list.size();
    for (int j = 0; j < list.size(); j++) {
        QHostInfo info = QHostInfo::fromName(list.at(j).toString());
        if (!info.addresses().isEmpty()) {
            auto address = info.addresses().first();
            logTrace() << "Got address: " << address;
            auto tcpServer = new QTcpServer();
            tcpServer->listen(address, port);
            if (not tcpServer->isListening()) {
                logDebug() << "Taken port on address:" << address << ":" << port;
                delete tcpServer;
                delete inter;
                return registerFreeTcpPort(10000 + rand);
            } else
                tcpServer->close();
            delete tcpServer;
        }
    }

    /* also perform additional check on 0.0.0.0 for services which are listening on multiple interfaces at once */
    auto tcpServer = new QTcpServer();
    tcpServer->listen(QHostAddress::Any, port);
    if (not tcpServer->isListening()) {
        logDebug() << "Taken port on 0.0.0.0:" << port;
        delete tcpServer;
        delete inter;
        return registerFreeTcpPort(10000 + rand);
    } else
        tcpServer->close();
    delete tcpServer;

    delete inter;
    return port;
}


/*
 *  Read file contents of text file
 */
string readFileContents(const QString& fileName) {
    QString lines = "";
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream stream(&f);
    stream.setCodec(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (!line.trimmed().isEmpty()) {
            lines += line + "\n";
            logTrace() << fileName << ":" << line;
        }
    }
    lines += "\n";
    f.close();
    return string(lines.toUtf8());
}


/*
 *  Parse string contents to Json value.
 */
Json::Value* parseJSON(const QString& filename) {
    Json::Reader reader; /* parse json file */
    auto root = new Json::Value();
    auto parsedSuccess = reader.parse(readFileContents(filename), *root, false);
    if (!parsedSuccess) {
        logError() << "JSON Parse Failure of file: " << filename;
        return root;
    }
    return root; /* return user side igniter first by default */
}
