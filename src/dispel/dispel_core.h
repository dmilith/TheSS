/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2013 - VerKnowSys
*/


#ifndef __DISPEL_CORE__
#define __DISPEL_CORE__


#include <QtCore>
#include <QUuid>
#include <stdexcept>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QtNetwork/QHostInfo>


/*
    example value for knownNodes file:

    {
      "known-nodes": [
          "192.168.1.2 {40a8e817-57de-4e5e-9806-2c1338a5079b}",
          "192.168.1.6 {a9d9dc51-10a4-ae4e-c41d-c9333af1af21}"
      ]
    }

*/

/*
    example value for defaultUUID file:

    {
        "node": "{40a8e817-57de-4e5e-9806-2c1338a5079b}",
    }

*/


#define DISPEL_NODE_PUBLISHER_PORT "12000" // XXX: multiplied by 1k for testing purposes
// #define DISPEL_NODE_SUBSCRIBER_PORT "14"
// #define DISPEL_NODE_FILE_SYNC_PORT "16"

#define DISPEL_NODE_PUBLISHER_ENDPOINT ("tcp://*:" DISPEL_NODE_PUBLISHER_PORT)
#define DISPEL_NODE_PUBLISHER_PAUSE 5000
#define DISPEL_NODE_SUBSCRIBER_ENDPOINT ("tcp://0.0.0.0:" DISPEL_NODE_PUBLISHER_PORT)

#define DISPEL_NODES_FILE "/.served-known-nodes.svd"
#define DISPEL_NODE_IDENTIFICATION_FILE "/.served-node-uuid.svd"
// #define DISPEL_NODE_KNOWN_NODES_DIR "/.svd-known-nodes/"
#define DISPEL_API_HEADER_LENGTH 39

#include "nzmqt/AbstractZmqBase.hpp"
#include "nzmqt/nzmqt.hpp"


void generateNodeUuid();
QString zmqVersion();
QString currentNodeUUID();
QMap<QString, QString> readNodesData();


#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../core/json_api.h"


#endif
