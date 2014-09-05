/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "cron_entry.h"
#include "utils.h"


SvdCrontab::SvdCrontab(const QString& cronEntry) {
    QString entry = cronEntry;
    auto cmd = cronEntry.split(CRON_DELIMITER);
    auto cronDSL = cmd.first().trimmed();
    auto commands = cmd.last().trimmed();
    if (cronDSL.isEmpty())
        cronDSL = "* * * * * " + QString(CRON_DELIMITER) + " " + commands;
    auto cronList = cronDSL.split(' ', QString::SkipEmptyParts);
    Q_FOREACH(auto elem, cronList)
        modes << NORMAL; /* normal mode is default for each element by default */

    logTrace() << "Parsing cron entry:" << cronEntry << "->" << cronList;
    for (int indx = 0; indx < cronList.length(); indx++) {
        auto value = cronList.at(indx);
        entries << value;
        logTrace() << "Crontab element:" << value;

        /* detect value type */
        if (value == "*") {
            logTrace() << "Wildcard mode for index:" << indx;
            modes[indx] = WILDCARD;
        }
        if (value.contains("/")) {
            logTrace() << "Periodic mode for index:" << indx;
            modes[indx] = PERIODIC;
        }
        if (value.contains("-")) {
            logTrace() << "Rango mode for index:" << indx;
            modes[indx] = RANGE;
        }
        if (value.contains(",")) {
            logTrace() << "Sequence mode for index:" << indx;
            modes[indx] = SEQUENCE;
        }
    }

    this->commands = commands;
    this->cronDSL = cronDSL;
    if (entry.startsWith("* * * * *") || entry.endsWith("/1 * * * *") || entry.endsWith("*/1 * * * *")) {
        logInfo() << "Cron continuous mode enabled.";
        this->continuous = true;
    }
}


SvdCronType SvdCrontab::workMode(int indx) {
    return modes.at(indx);
}


bool SvdCrontab::check(int currentTimeValue, int indx) {
    switch (workMode(indx)) {
        case NORMAL: {
                if (currentTimeValue == entries.at(indx).toInt()) {
                    logTrace() << "Matched cron minute:" << currentTimeValue;
                } else
                    return false;
            } break;

        case WILDCARD: {
                logTrace() << "Matched (wildcard) cron minute:" << currentTimeValue;
            } break;

        case PERIODIC: {
                auto value = entries.at(indx).split("/");
                int first = value.at(1).toInt();
                if (first == 0) {
                    logError() << "Wrong periodic value:" << entries.at(indx) << " Modulo by 0 is an invalid operation!";
                    return false;
                }
                if (currentTimeValue % first == 0) {
                    logDebug() << "Matched cron periodic minute:" << currentTimeValue;
                } else
                    return false;
            } break;

        case SEQUENCE: {
                auto value = entries.at(indx).split(",");
                bool pass = false;
                for (int i = 0; i < value.length(); i++) {
                    if (currentTimeValue == value.at(i).toInt())
                        pass = true;
                }
                if (not pass)
                    return false;
            } break;

        case RANGE: {
                auto value = entries.at(indx).split("-");
                bool pass = false;
                int first = value.at(0).toInt();
                int second = value.at(1).toInt();
                if (second > first)
                    for (int i = first; i <= second; i++) {
                        if (currentTimeValue == i)
                            pass = true;
                    }
                else {
                    logError() << "Wrong range value in crontab:" << entries.at(indx);
                    return false;
                }
                if (not pass)
                    return false;
            } break;

        default:
            logError() << "Invalid work mode in cron entry!";
            return false;

    }

    return true;
}


bool SvdCrontab::isContinuous() {
    return continuous;
}


bool SvdCrontab::cronMatch(const QDateTime& now) {

    /* check for empty input data */
    if (entries.length() == 0 or modes.length() == 0) {
        logDebug() << "Empty entries? Return true - it's wildcard case equivalent of entry '* * * * * ?!' (run command each minute).";
        return true;
    }

    for (int indx = 0; indx < entries.length(); indx++) {

        switch (indx) {

            /* minutes */
            case 0: {
                if (not check(now.time().minute(), indx))
                    return false;
            } break;

            /* hours */
            case 1: {
                if (not check(now.time().hour(), indx))
                    return false;
            } break;

            /* dayOfMonth */
            case 2: {
                if (not check(now.date().day(), indx))
                    return false;
            } break;

            /* month */
            case 3: {
                if (not check(now.date().month(), indx))
                    return false;
            } break;

            /* dayOfWeek */
            case 4: {
                if (not check(now.date().dayOfWeek(), indx))
                    return false;
            } break;

            /* commands */
            case 5: {
                if (this->commands.isEmpty())
                    logError() << "Empty commands given, or cron entry format is invalid. (Might be forgotten \"" << CRON_DELIMITER << "\" at end of cronEntry)";
            } break;

            /* something's wrong with entry? */
            default:
                logError() << "Bad cron entry matcher!";
                return false;
                break;

        }
    }

    logDebug() << "Cron matches:" << commands;
    return true;
}


void SvdCrontab::pp() {
    for (int i = 0; i < entries.length(); i++) {
        logTrace() << "Crontab entry mode:" << modes.at(i) << "Crontab entry contents:" << entries.at(i);
    }
}
