
/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2013 - VerKnowSys
*/


#include "dispel_core.h"


QMap<QString,QString> allKnownNodeUUIDs() {
    QMap<QString,QString> nodes; // = parseJSON(readFileContents(DISPEL_NODE_IDENTIFICATION_FILE));
    if (nodes.isEmpty()) {
        logFatal() << "No nodes available found! Something bad happened. I will die now.";
    }
    return nodes;
}


QString currentNodeUUID() {
    const QMap<QString,QString> nodeID; // = parseJSON(readFileContents(DISPEL_NODE_IDENTIFICATION_FILE));

    auto value = DEFAULT_LOCAL_ADDRESS;
    if (nodeID.keys().length() == 0) {
        logDebug() << nodeID.keys();
        logFatal() << "No default node identifier found. No identifier for default IP:" << DEFAULT_LOCAL_ADDRESS;
    } else
        Q_FOREACH(auto key, nodeID.keys()) {
            logDebug() << "key:" << key;
            if (key == DEFAULT_LOCAL_ADDRESS) {
                auto result = nodeID.value(key);
                logInfo() << "Found local node identifier:" << result;
                return result;
            }
        }

    return value; /* NOTE: will return uuid for 127.0.0.1 host (DEFAULT_LOCAL_ADDRESS) as current system */
}


/*
 *  Parse string contents to Json value.
 */
// QMap<QString,QString> parseJSON(const QString& contents) {
    // Json::Reader reader; /* parse json file */
    // auto root = new Json::Value();

    // auto parsedSuccess = reader.parse(contents.toUtf8().data(), *root, false);
    // if (!parsedSuccess) {
    //     logError() << "JSON Parse Failure of content:" << contents;
    //     root = NULL; /* this indicates that json parser failed to get data from igniter */
    //     return QMap<QString,QString>();
    // }

    /* checking schema rules for Igniter format */
    // QMap<QString,QString> mapOfKnownUUIDs;

    // const QString knownNodesIdentifier = "knownNodes";
    // auto values = (*root)[knownNodesIdentifier.toStdString()];
    // if (not values.isArray() or values.isInt() or values.isBool() or values.isNumeric()) {
    //     logError() << "JSON Type: Array - Failed in content:" << contents << "In field:" << knownNodesIdentifier;
    // } else {
    //     int amount = values.size();
    //     logDebug() << "Array with" << amount << "objects";

    //     for (uint index = 0; index < (*root)[knownNodesIdentifier.toStdString()].size(); ++index ) {
    //         try {
    //             Json::Value key, value;
    //             QString ip = (*root)[knownNodesIdentifier.toStdString()][index].get((uint)0, &key).asCString();
    //             QString uuid = (*root)[knownNodesIdentifier.toStdString()][index].get((uint)1, &value).asCString();
    //             logDebug() << ip << "->" << uuid;
    //             mapOfKnownUUIDs.insert(ip, uuid);

    //         } catch (std::exception &e) {
    //             logError() << "Exception while parsing known nodes!";
    //         }
    //     }
    // }
    // root = NULL;
    // return mapOfKnownUUIDs;
// }


QString zmqVersion() {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    return QString::number(major) + "." + QString::number(minor) + "." + QString::number(patch);
}


QStringList getCurrentNodeAddresses() {
    QStringList result;
    QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
    if (!info.addresses().isEmpty()) {
        QHostAddress address = info.addresses().first();
        logDebug() << "Trying with address:" << address;
        result << address.toString();
    }
    return result;
}


// QString readOrGenerateNodeUuid() {
//     /* permission check */
//     QString dirOnPath = QFileInfo(DISPEL_NODE_IDENTIFICATION_FILE).dir().absolutePath();
//     if (getuid() == 0) {
//         logDebug() << "Dir on path:" << dirOnPath;
//         if (not QFile::permissions(dirOnPath).testFlag(QFile::ExeOwner)) {
//             logFatal() << "Insufficient permissions to traverse through directory:" << dirOnPath << ":" << QFile::permissions(dirOnPath).testFlag(QFile::ExeOwner) << "," << QFile::permissions(dirOnPath).testFlag(QFile::ExeUser) << "," << QFile::permissions(dirOnPath).testFlag(QFile::ExeOther);
//         } else {
//             logDebug() << "Permissions granted to traverse through directory:" << dirOnPath;
//         }
//         if ((not QFile::permissions(DISPEL_NODE_IDENTIFICATION_FILE).testFlag(QFile::ReadOwner)) and
//                  QFile::exists(DISPEL_NODE_IDENTIFICATION_FILE)) {
//             logFatal() << "Can't read Node ID from file:" << DISPEL_NODE_IDENTIFICATION_FILE << "Check access rights to this file for current user and try again!";
//         } else {
//             logDebug() << "Permissions granted to read file:" << DISPEL_NODE_IDENTIFICATION_FILE;
//         }
//     } else {
//         logWarn() << "Launching Dispel as non root user. I assume it's just development build to test something out.";
//         QString develNodeName = "{devel-node-uuid}";
//         logWarn() << "Temporarely set Node ID to:" << develNodeName;
//         return develNodeName;
//     }

//     QString content = "\0";
//     if (QFile::exists(QString(DISPEL_NODE_IDENTIFICATION_FILE))) {
//         logDebug() << "Permissions to read file:" << DISPEL_NODE_IDENTIFICATION_FILE << ":" << QFile::permissions(DISPEL_NODE_IDENTIFICATION_FILE).testFlag(QFile::ReadOwner) << "," << QFile::permissions(DISPEL_NODE_IDENTIFICATION_FILE).testFlag(QFile::ReadUser) << "," << QFile::permissions(DISPEL_NODE_IDENTIFICATION_FILE).testFlag(QFile::ReadOther);

//         uint tmpFileSize = QFile(DISPEL_NODE_IDENTIFICATION_FILE).size();
//         if (tmpFileSize != UUID_CORRECT_LENGTH) {
//             QString error = "\0";
//             error += QString("Found malformed, or incorrect Node ID:" ) + readFileContents(DISPEL_NODE_IDENTIFICATION_FILE).trimmed() + QString(" with length:") + QString::number(tmpFileSize) + QString(" Should be ") + QString::number(UUID_CORRECT_LENGTH);

//             notification(error, "Dispel", ERROR);
//             error = "Node ID destroyed on machine XXX:TODO:FIXME, and will be recreated!";
//             notification(error, "Dispel", ERROR);
//             QFile::remove(DISPEL_NODE_IDENTIFICATION_FILE);
//             return readOrGenerateNodeUuid();
//         }

//         content = readFileContents(DISPEL_NODE_IDENTIFICATION_FILE).trimmed();
//         notification("New ServeD Node, with ID: " + content + " has joined the party.", "Dispel", NOTIFY);

//     } else {
//         content = QUuid().createUuid().toString().trimmed();
//         QString info = QString("ServeD Node isn't defined on this host. Deploying new Node ID: ") + content;
//         notification(info, "Dispel", NOTIFY);
//         writeToFile(DISPEL_NODE_IDENTIFICATION_FILE, content);
//     }
//     return content;
// }
