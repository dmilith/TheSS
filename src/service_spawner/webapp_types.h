/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __WEBAPP_TYPES__
#define __WEBAPP_TYPES__


#include "utils.h"


enum WebAppTypes {
    StaticSite          = 0x01,
    UnicornRailsSite    = 0x02,
    RailsSite           = 0x03,
    NodeSite            = 0x04,

    NoType              = 0x05 /* NOTE: this one *must* be always last type */
};


class WebAppTypeDetector {

    WebAppTypes appType = NoType;

    public:
        WebAppTypeDetector(const QString& path);
        WebAppTypes getType();
        QString typeName;
};


#endif
