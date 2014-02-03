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
    RubySite            = 0x01,
    NodeSite            = 0x02,
    PhpSite             = 0x03,

    StaticSite          = 0x05 /* NOTE: this one *must* be always last type */
};

enum WebDatastore {
    Postgresql          = 0x01,
    Mysql               = 0x02,
    Mongo               = 0x03,
    Redis               = 0x04,
    ElasticSearch       = 0x05,
    Sphinx              = 0x06,

    NoDB                = 0x07
};

class WebAppTypeDetector {

    WebAppTypes appType = StaticSite;

    public:
        WebAppTypeDetector(const QString& path);
        WebAppTypes getType();
        QString typeName;
};


#endif
