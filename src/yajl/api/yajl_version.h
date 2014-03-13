#ifndef YAJL_VERSION_H_
#define YAJL_VERSION_H_

#include "yajl_common.h"

#define YAJL_MAJOR "2"
#define YAJL_MINOR "0"
#define YAJL_MICRO "13cf23655c8f917c3f487800d7bb397b7cbc432a"

#define YAJL_VERSION (YAJL_MAJOR "." YAJL_MINOR "." YAJL_MICRO)

#ifdef __cplusplus
extern "C" {
#endif

extern const char* YAJL_API yajl_version(void);

#ifdef __cplusplus
}
#endif

#endif /* YAJL_VERSION_H_ */

