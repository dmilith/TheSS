/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "cron_entry.h"
#include "utils.h"


SvdCrontab::SvdCrontab(const QString& cronEntry, const QString& commands) {
    auto cronList = cronEntry.split(' ', QString::SkipEmptyParts);
    logDebug() << "Parsing cron entry:" << cronEntry << "->" << cronList;
    for (int indx = 0; indx < cronList.size(); indx++) {
        auto value = cronList.at(indx);
        logTrace() << "Crontab element:" << value;

        /* wildcard case */
        bool wildcard = false;
        bool periodic = false;
        if (value == "*") {
            logTrace() << "Wildcard for index:" << indx;
            wildcard = true;
        }
        auto tmp = value.split("*/");
        if (value.contains("*/")) {
            logDebug() << "Periodic cron value:" << value;
            logWarn() << "INTERESTING VALUE:" << tmp.at(0) << "SECOND:" << tmp.at(1);
            periodic = true;
        }
        if (value.contains("/")) {
            tmp = value.split("/");
            logDebug() << "Periodic cron value:" << value;
            logWarn() << "INTERESTING VALUE:" << tmp.at(0) << "SECOND:" << tmp.at(1);
            periodic = true;
        }

        switch (indx) {

            /* minutes */
            case 0:
                if (not wildcard) this->minute = value.toInt();
                if (periodic) this->minuteFraction = tmp.at(1).toInt();
                break;

            /* hour */
            case 1:
                if (not wildcard) this->hour = value.toInt();
                if (periodic) this->hourFraction = tmp.at(1).toInt();
                break;

            /* dayOfMonth */
            case 2:
                if (not wildcard) this->dayOfMonth = value.toInt();
                if (periodic) this->dayOfMonthFraction = tmp.at(1).toInt();
                break;

            /* month */
            case 3:
                if (not wildcard) this->month = value.toInt();
                if (periodic) this->monthFraction = tmp.at(1).toInt();
                break;

            /* dayOfWeek */
            case 4:
                if (not wildcard) this->dayOfWeek = value.toInt();
                if (periodic) this->dayOfWeekFraction = tmp.at(1).toInt();
                break;

            /* commands */
            case 5:
                if (value == "?")
                    this->commands = commands;
                break;

            /* something's wrong with entry? */
            default:
                logError() << "Bad cron entry format!";
                break;

        }
    }
}


bool SvdCrontab::cronMatch() {
    auto now = QDateTime::currentDateTime();

    QList<int> cronData = QList<int>();
    QList<int> fractionCronData = QList<int>();
    cronData << this->minute << this->hour << this->dayOfMonth << this->month << this->dayOfWeek;
    fractionCronData << this->minuteFraction << this->hourFraction << this->dayOfMonthFraction << this->monthFraction << this->dayOfWeekFraction;

    for (int indx = 0; indx < cronData.length(); indx++) {
        bool wildcard = false;
        if (cronData.at(indx) == -1)
            wildcard = true;

        switch (indx) {

            /* minutes */
            case 0:
                if (minuteFraction == -1) {
                    if (not wildcard) {
                        if (this->minute == cronData.at(indx)) {
                            logTrace() << "Matched cron minute:" << this->minute;
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron minute:" << this->minute;
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (this->minute / fractionCronData.at(indx) == cronData.at(indx)) {
                        logDebug() << "Matched fraction time:" << this->minute / fractionCronData.at(indx) << "vs" << cronData.at(indx);
                    } else
                        return false;
                }
                break;

            /* hour */
            case 1:
                if (hourFraction == -1) {
                    if (not wildcard) {
                        if (this->hour == cronData.at(indx)) {
                            logTrace() << "Matched cron hour:" << this->hour;
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron hour:" << this->hour;
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (this->hour / fractionCronData.at(indx) == cronData.at(indx)) {
                        logDebug() << "Matched fraction time:" << this->hour / fractionCronData.at(indx) << "vs" << cronData.at(indx);
                    } else
                        return false;
                }
                break;

            /* dayOfMonth */
            case 2:
                if (dayOfMonthFraction == -1) {
                    if (not wildcard) {
                        if (this->dayOfMonth == cronData.at(indx)) {
                            logTrace() << "Matched cron dayOfMonth:" << this->dayOfMonth;
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron dayOfMonth:" << this->dayOfMonth;
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (this->dayOfMonth / fractionCronData.at(indx) == cronData.at(indx)) {
                        logDebug() << "Matched fraction time:" << this->dayOfMonth / fractionCronData.at(indx) << "vs" << cronData.at(indx);
                    } else
                        return false;
                }
                break;

            /* month */
            case 3:
                if (monthFraction == -1) {
                    if (not wildcard) {
                        if (this->month == cronData.at(indx)) {
                            logTrace() << "Matched cron month:" << this->month;
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron month:" << this->month;
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (this->month / fractionCronData.at(indx) == cronData.at(indx)) {
                        logDebug() << "Matched fraction time:" << this->month / fractionCronData.at(indx) << "vs" << cronData.at(indx);
                    } else
                        return false;
                }
                break;

            /* dayOfWeek */
            case 4:
                if (dayOfWeekFraction == -1) {
                    if (not wildcard) {
                        if (this->dayOfWeek == cronData.at(indx)) {
                            logTrace() << "Matched cron dayOfWeek:" << this->dayOfWeek;
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron dayOfWeek:" << this->dayOfWeek;
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (this->dayOfWeek / fractionCronData.at(indx) == cronData.at(indx)) {
                        logDebug() << "Matched fraction time:" << this->dayOfWeek / fractionCronData.at(indx) << "vs" << cronData.at(indx);
                    } else
                        return false;
                }
                break;

            /* something's wrong with entry? */
            default:
                logError() << "Bad cron entry matcher!";
                break;

        }

    }

    logDebug() << "Cronmatch check:" <<
        now.time().minute() << "vs" << this->minute <<
        now.time().hour() << "vs" << this->hour <<
        now.date().day() << "vs" << this->dayOfMonth <<
        now.date().month() << "vs" << this->month <<
        now.date().dayOfWeek() << "vs" << this->dayOfWeek;

    if (now.time().minute() == this->minute and
        now.time().hour() == this->hour and
        now.date().day() == this->dayOfMonth and
        now.date().month() == this->month and
        now.date().dayOfWeek() == this->dayOfWeek) {
        logDebug() << "MATCHED!";

        return true;

    } else {

        return false;
    }
}


QString SvdCrontab::pp() {
    return "Cron minute:" + QString::number(this->minute) + ", Cron hour:" + QString::number(this->hour) + ", Cron dayOfMonth:" + QString::number(this->dayOfMonth) + ", Cron month:" + QString::number(this->month) + ", Cron dayOfWeek:" + QString::number(this->dayOfWeek) + ", Cron commands:" + this->commands;
}
