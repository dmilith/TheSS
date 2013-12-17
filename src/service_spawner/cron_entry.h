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


enum SvdCronType {
    NORMAL,     // X
    SEQUENCE,   // X,Y,Z
    RANGE,      // X-Z
    WILDCARD,   // *
    PERIODIC    // X/Y
};


class SvdCrontab {

    public:
        /*
        possible values:
            -1 => wildcard value
         */
        QStringList entries;
        QList<SvdCronType> modes;
        QString commands;
        bool continuous = false;

        SvdCrontab(const QString& cronEntry, const QString& commands);
        bool check(int currentTimeValue, int indx);
        bool cronMatch(const QDateTime& now = QDateTime::currentDateTime());
        bool isContinuous();
        SvdCronType workMode(int indx);
        void pp();

};


#endif
