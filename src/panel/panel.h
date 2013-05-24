/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __SVDPANEL_H__
#define __SVDPANEL_H__


#include "../globals/globals.h"
#include "../service_spawner/logger.h"
#include "../service_spawner/service_config.h"
#include "../service_spawner/service_watcher.h"
#include "../service_spawner/user_watcher.h"
#include "../service_spawner/utils.h"

#include <QtCore>

#include <curses.h>
// #include <panel.h>
// #include <menu.h>
// #include <form.h>


void printStatus(QString status) {
    attron(COLOR_PAIR(5));
    mvprintw(1, 1, "status: %-130s", status.toUtf8().data()); // XXX: hardcoded max length of status content
    attroff(COLOR_PAIR(5));
}


QFileInfoList getApps(QDir home) {
    return home.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoDot);
}


int kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}


#endif
