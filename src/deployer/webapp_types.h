/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __WEBAPP_TYPES__
#define __WEBAPP_TYPES__


#include "../service_spawner/utils.h"


enum WebAppTypes {
    StaticSite          = 0x01,
    RubySite            = 0x02,
    NodeSite            = 0x03,

    NoType              = 0x05 /* NOTE: this one *must* be always last type */
};

enum WebDatabase {
    Postgresql          = 0x01,
    Mysql               = 0x02,

    NoDB                = 0x05
};

class WebAppTypeDetector {

    WebAppTypes appType = NoType;

    public:
        WebAppTypeDetector(const QString& path);
        WebAppTypes getType();
        QString typeName;
};


#endif
