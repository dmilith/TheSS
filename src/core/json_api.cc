/**
 *  @author dmilith
 *
 *   JSON API for igniters and configs.
 *   © 2014 - VerKnowSys
 *
 */


#include "logger.h"
#include "json_api.h"


void JSONAPI::getTreeNode(yajl_val nodeDefault, yajl_val nodeRoot, const char* element, yajl_val* v, yajl_val* w) {
    /* building paths */
    char* elem = strdup(element);
    char* result = NULL;
    char delims[] = "/";
    result = strtok(elem, delims);
    const char *path[MAX_DEPTH];
    int i = 0;
    for (i = 0; result != NULL; i++) {
        path[i] = (const char*)malloc(strlen(result) + 1) ; //new char[strlen(result)];
        strncpy((char*)path[i], result, strlen(result) + 1);
        path[strlen(result)] = DEFAULT_ZERO_CHAR;
        result = strtok( NULL, delims );
    }
    path[i] = DEFAULT_ZERO_CHAR;

    *v = yajl_tree_get(nodeDefault, path, yajl_t_any);
    *w = NULL;
    if (nodeRoot)
        *w = yajl_tree_get(nodeRoot, path, yajl_t_any);
    for (int j = 0; j <= i; j++) {
        delete[] path[j];
    }
    free(elem);
}


QStringList JSONAPI::getArray(yajl_val nodeDefault, yajl_val nodeRoot, const char* element) {
    yajl_val v, w;
    getTreeNode(nodeDefault, nodeRoot, element, &v, &w);
    QStringList buf;

    if (v and YAJL_IS_ARRAY(v)) {
        int len = v->u.array.len;
        for (int i = 0; i < len; ++i) { /* gather default list */
            yajl_val obj = v->u.array.values[i];
            buf << YAJL_GET_STRING(obj);
        }
    }
    if (w and YAJL_IS_ARRAY(w)) {
        int len = w->u.array.len;
        for (int i = 0; i < len; ++i) { /* gather igniter list */
            yajl_val obj = w->u.array.values[i];
            buf << YAJL_GET_STRING(obj);
        }
    }
    return buf;
}


double JSONAPI::getDouble(yajl_val nodeDefault, yajl_val nodeRoot, const char* element) {
    yajl_val v, w;
    getTreeNode(nodeDefault, nodeRoot, element, &v, &w);

    /* user igniter has priority */
    if (w and YAJL_IS_DOUBLE(w)) {
        double lng = YAJL_GET_DOUBLE(w);
        return lng;
    }
    if (v and YAJL_IS_DOUBLE(v)) {
        double lng = YAJL_GET_DOUBLE(v);
        return lng;
    }
    logError() << "Not a double:" << element;
    return 0.0;
}


long long JSONAPI::getInteger(yajl_val nodeDefault, yajl_val nodeRoot, const char* element) {
    yajl_val v, w;
    getTreeNode(nodeDefault, nodeRoot, element, &v, &w);

    /* user igniter has priority */
    if (w and YAJL_IS_INTEGER(w)) {
        long long lng = YAJL_GET_INTEGER(w);
        // logTrace() << "Parsed Root Integer:" << QString::number(lng);
        return lng;
    }
    if (v and YAJL_IS_INTEGER(v)) {
        long long lng = YAJL_GET_INTEGER(v);
        // logTrace() << "Parsed Default Integer:" << QString::number(lng);
        return lng;
    }
    logError() << "Not a integer:" << element;
    return 0L;
}


bool JSONAPI::getBoolean(yajl_val nodeDefault, yajl_val nodeRoot, const char* element) {
    yajl_val v, w;
    getTreeNode(nodeDefault, nodeRoot, element, &v, &w);

    if (w and (YAJL_IS_TRUE(w) or YAJL_IS_FALSE(w))) {
        if (YAJL_IS_TRUE(w)) {
            return true;
        } else {
            return false;
        }
    }
    if (v and (YAJL_IS_TRUE(v) or YAJL_IS_FALSE(v))) {
        if (YAJL_IS_TRUE(v)) {
            return true;
        } else {
            return false;
        }
    }

    logError() << "No such boolean node:" << element;
    return false;
}


QString JSONAPI::getString(yajl_val nodeDefault, yajl_val nodeRoot, const char* element) {
    yajl_val v, w;
    getTreeNode(nodeDefault, nodeRoot, element, &v, &w);

    if (w and YAJL_IS_STRING(w)) {
        return YAJL_GET_STRING(w);
    }
    if (v and YAJL_IS_STRING(v)) {
        return YAJL_GET_STRING(v);
    }

    logError() << "No such string node:" << element;
    return "";
}

