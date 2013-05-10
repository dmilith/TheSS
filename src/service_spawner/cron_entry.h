/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef __CRON_ENTRY_H__
#define __CRON_ENTRY_H__


#include "QtCore"
#include "../globals/globals.h"


 class SvdCrontab {

    public:
        /*
        possible values:
            -1 => wildcard value
         */
        int minute = -1, minuteFraction = -1,
            hour = -1, hourFraction = -1,
            dayOfMonth = -1, dayOfMonthFraction = -1,
            month = -1, monthFraction = -1,
            dayOfWeek = -1, dayOfWeekFraction = -1;
        QString commands;

        SvdCrontab(const QString& cronEntry, const QString& commands);
        bool cronMatch();
        QString pp();

};


#endif
