
/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2013 - VerKnowSys
*/


#include "dispel_core.h"


QMap<QString, QString> readNodesData() {
    char errbuf[1024];
    auto nodesFileContent = readFileContents(getHomeDir() + DISPEL_NODES_FILE).toUtf8();
    auto knownNodes = yajl_tree_parse(nodesFileContent, errbuf, sizeof(errbuf));
    if (QString(errbuf).length() > 0) {
        logError() << "Error in nodes file:" << DISPEL_NODES_FILE << "::" << errbuf;
    }
    QStringList nodes = JSONAPI::getArray(knownNodes, NULL, DISPEL_NODES_ENTRY);

    QMap<QString, QString> buf;
    Q_FOREACH(QString node, nodes) {
        auto pair = node.split(" ");
        auto addr = pair.first();
        auto uuid = pair.last();
        buf.insert(addr, uuid);
        logInfo() << "Known node:" << addr << "->" << uuid;
    }
    return buf;
}


QString currentNodeUUID() {
    generateNodeUuid();
    char errbuf[1024];
    auto nodesFileContent = readFileContents(getHomeDir() + DISPEL_NODE_IDENTIFICATION_FILE).toUtf8();
    auto currentNode = yajl_tree_parse(nodesFileContent, errbuf, sizeof(errbuf));
    if (QString(errbuf).length() > 0) {
        logError() << "Error in identification file:" << getHomeDir() + DISPEL_NODE_IDENTIFICATION_FILE << "::" << errbuf;
    }
    QString node = JSONAPI::getString(currentNode, NULL, DISPEL_NODE_ENTRY);
    yajl_tree_free(currentNode);
    return node; /* NOTE: will return uuid for 127.0.0.1 host (DEFAULT_LOCAL_ADDRESS) as current system */
}


QString zmqVersion() {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    return QString::number(major) + "." + QString::number(minor) + "." + QString::number(patch);
}


void generateNodeUuid() {
    QString dirOnPath = getHomeDir() + DISPEL_NODE_IDENTIFICATION_FILE;
    QString content = "";
    if (QFile::exists(dirOnPath)) {
        logInfo() << "Found node identificator in file:" << dirOnPath;
    } else {
        auto uuid = QUuid().createUuid().toString().trimmed();
        logInfo() << "ServeD Node isn't defined on this host. Defining new node:" << uuid;
        content = "{\"node\": \"" + uuid + "\"}";
        writeToFile(getHomeDir() + DISPEL_NODE_IDENTIFICATION_FILE, content);
        if (not QFile::exists(getHomeDir() + DISPEL_NODES_FILE)) {
            logInfo() << "ServeD list undefined. Creating empty.";
            writeToFile(getHomeDir() + DISPEL_NODES_FILE, "{}");
        }
    }
}
