/**
 *  @author dmilith
 *
 *   JSON API for igniters and configs.
 *   © 2014 - VerKnowSys
 *
 */


#ifndef __JSON_API__
#define __JSON_API__


#include "../yajl/api/yajl_tree.h"

#include <QObject>
#include <QStringList>


#define ZERO_CHAR (char *)0
#define MAX_DEPTH 64 /* max supported json tree depth */


class JSONAPI {

    public:
        static void getTreeNode(yajl_val nodeDefault, yajl_val nodeRoot, const char* element, yajl_val* v, yajl_val* w);
        static QString getString(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);
        static QStringList getArray(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);
        static bool getBoolean(const yajl_val nodeDefault, const yajl_val nodeRoot, const char* element);
        static long long getInteger(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);
        static double getDouble(yajl_val nodeDefault, yajl_val nodeRoot, const char* element);

};


#endif
