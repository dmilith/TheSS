/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include "death_watch.h"


void deathWatch(pid_t aPid, uint signal) {
    if (aPid == 0) {
        logDebug() << "Given pid 0, it usually means that no process to kill, cause it's already dead.";
        return;
    }

    if (aPid == 1) {
        logError() << "You can't put a death watch on pid 1.";
        return;
    }

    if (kill(aPid, 0) == 0) {
        logDebug() << "Process with pid:" << aPid << "still exists in process list.";
    } else {
        logInfo() << "Process with pid:" << aPid << "was interruped.";
        return;
    }

    QString level;
    switch (signal) {
        case SIGCONT:
            level = "CONT";
            break;

        case SIGINT:
            level = "INT";
            break;

        case SIGTERM:
            level = "TERM";
            break;

        case SIGKILL:
            level = "KILL";
            break;
    }
    logDebug() << "Death watch for pid:" << aPid << "Signal level:" << level;

    if (kill(aPid, signal) == 0) {
        if (kill(aPid, 0) != 0) {
            logInfo() << "Process with pid:" << aPid << "was interruped.";
            return;
        }
    }

    if (signal != SIGCONT)
        usleep(DEFAULT_DEATH_WATCHER_PAUSE);

    switch (signal) {

        case SIGCONT: {
            deathWatch(aPid, SIGINT);
        } break;

        case SIGINT: {
            deathWatch(aPid, SIGTERM);
        } break;

        case SIGTERM: {
            deathWatch(aPid, SIGKILL);
        } break;

        case SIGKILL: {
            logError() << "Process is ignoring KILL signal! Something is crashed badly! Still trying for pid:" << aPid;
            deathWatch(aPid, SIGKILL);
        } break;

    }
}
