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
    RubySite = 0,
    NodeSite,
    PhpSite,
    StaticSite /* NOTE: this one *must* be always last type */
};

enum WebDatastore {
    Postgresql = 0,
    Mysql,
    Mongo,
    Redis,
    ElasticSearch,
    Sphinx,
    NoDB
};

class WebAppTypeDetector {

    WebAppTypes appType = StaticSite;

    public:
        WebAppTypeDetector(const QString& path);
        WebAppTypes getType();
        QString typeName;
};


#endif
