#ifndef CUTELOGGER_GLOBAL_H
#define CUTELOGGER_GLOBAL_H

// #define DEFAULT_STRING_CODEC "UTF-8"

#include "../globals/globals.h"
#include <QtCore/qglobal.h>

#if defined(CUTELOGGER_LIBRARY)
#  define CUTELOGGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CUTELOGGERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CUTELOGGER_GLOBAL_H
