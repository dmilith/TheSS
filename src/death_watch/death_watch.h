/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef __DEATH_WATCH_H__
#define __DEATH_WATCH_H__

#include "../globals/globals.h"
#include "../service_spawner/utils.h"

#include <QtCore>


#define DEFAULT_DEATH_WATCHER_PAUSE 5000000 /* 5 seconds by default */

/**
 * Function performs endless loop, sending signals to process until it dies.
 * @param process a process object
 */
void deathWatch(pid_t pid, uint signal = SIGCONT);


#endif
