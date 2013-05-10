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
    for (int indx = 0; indx < cronList.length(); indx++) {
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
            logTrace() << "INTERESTING VALUE:" << tmp.at(0) << "SECOND:" << tmp.at(1);
            periodic = true;
        }
        if (value.contains("/")) {
            tmp = value.split("/");
            logDebug() << "Periodic cron value:" << value;
            logTrace() << "INTERESTING VALUE:" << tmp.at(0) << "SECOND:" << tmp.at(1);
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


bool SvdCrontab::cronMatch(const QDateTime& now) {

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
                        if (now.time().minute() == cronData.at(indx)) {
                            logTrace() << "Matched cron minute:" << now.time().minute();
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron minute:" << now.time().minute();
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (now.time().minute() % fractionCronData.at(indx) == 0) {
                        logDebug() << "Matched fraction time:" << now.time().minute();
                    } else
                        return false;
                }
                break;

            /* hour */
            case 1:
                if (hourFraction == -1) {
                    if (not wildcard) {
                        if (now.time().hour() == cronData.at(indx)) {
                            logTrace() << "Matched cron hour:" << now.time().hour();
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron hour:" << now.time().hour();
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (now.time().hour() % fractionCronData.at(indx) == 0) {
                        logDebug() << "Matched fraction time:" << now.time().hour();
                    } else
                        return false;
                }
                break;

            /* dayOfMonth */
            case 2:
                if (dayOfMonthFraction == -1) {
                    if (not wildcard) {
                        if (now.date().day() == cronData.at(indx)) {
                            logTrace() << "Matched cron dayOfMonth:" << now.date().day();
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron dayOfMonth:" << now.date().day();
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (now.date().day() % fractionCronData.at(indx) == 0) {
                        logDebug() << "Matched fraction time:" << now.date().day();
                    } else
                        return false;
                }
                break;

            /* month */
            case 3:
                if (monthFraction == -1) {
                    if (not wildcard) {
                        if (now.date().month() == cronData.at(indx)) {
                            logTrace() << "Matched cron month:" << now.date().month();
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron month:" << now.date().month();
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (now.date().month() % fractionCronData.at(indx) == 0) {
                        logDebug() << "Matched fraction time:" << now.date().month();
                    } else
                        return false;
                }
                break;

            /* dayOfWeek */
            case 4:
                if (dayOfWeekFraction == -1) {
                    if (not wildcard) {
                        if (now.date().dayOfWeek() == cronData.at(indx)) {
                            logTrace() << "Matched cron dayOfWeek:" << now.date().dayOfWeek();
                        } else
                            return false;
                    } else {
                        logTrace() << "Matched (wildcard) cron dayOfWeek:" << now.date().dayOfWeek();
                    }
                } else { // cron format: */ABC, where ABC is fraction value
                    if (now.date().dayOfWeek() % fractionCronData.at(indx) == 0) {
                        logDebug() << "Matched fraction time:" << now.date().dayOfWeek();
                    } else
                        return false;
                }
                break;

            /* something's wrong with entry? */
            default:
                logError() << "Bad cron entry matcher!";
                return false;
                break;

        }
    }

    logDebug() << "Cron entry datetime match:" <<
        now.time().minute() << "vs" << this->minute <<
        now.time().hour() << "vs" << this->hour <<
        now.date().day() << "vs" << this->dayOfMonth <<
        now.date().month() << "vs" << this->month <<
        now.date().dayOfWeek() << "vs" << this->dayOfWeek;

    if (this->commands.isEmpty())
        logError() << "Empty commands given, or cron entry format is invalid. (Might be forgotten \"?\" at end of cronEntry)";
    return true;
}


QString SvdCrontab::pp() {
    return "Cron minute:" + QString::number(this->minute) + ", Cron hour:" + QString::number(this->hour) + ", Cron dayOfMonth:" + QString::number(this->dayOfMonth) + ", Cron month:" + QString::number(this->month) + ", Cron dayOfWeek:" + QString::number(this->dayOfWeek) + ", Cron commands:" + this->commands;
}
