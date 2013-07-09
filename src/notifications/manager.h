/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __SNOTIFICATIONS_MANAGER_H__
#define __SNOTIFICATIONS_MANAGER_H__


#include <QtCore>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../service_spawner/utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>


#define NOTIFICATION_LEVEL_ERROR    0
#define NOTIFICATION_LEVEL_WARNING  1
#define NOTIFICATION_LEVEL_NOTICE   2

struct Notification {
    int level;
    QString content;
    QDateTime time;
};

#endif
