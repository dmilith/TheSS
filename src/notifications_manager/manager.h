/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __SNOTIFICATIONS_MANAGER_H__
#define __SNOTIFICATIONS_MANAGER_H__


#define NOTIFICATION_LEVEL_ERROR    0
#define NOTIFICATION_LEVEL_WARNING  1
#define NOTIFICATION_LEVEL_NOTICE   2

struct Notification {
    int level;
    QString content;
    QDateTime time;
};

#endif
