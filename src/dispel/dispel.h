/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef __DISPEL_H__
#define __DISPEL_H__


#include <QtCore>

#ifdef __linux__
    #include <linux/socket.h>
#else
    #include <sys/ucred.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dispel_core.h"


#endif
