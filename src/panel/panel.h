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
#include "../service_spawner/utils.h"

#include <QtCore>
#include <curses.h>


QStringList availableServices() {
    auto userEntries = QDir(getenv("HOME") + QString(DEFAULTUSERIGNITERSDIR)).entryList(QDir::Files);
    auto standardEntries = QDir(QString(DEFAULTSOFTWARETEMPLATESDIR)).entryList(QDir::Files);
    auto rootEntries = QStringList();
    if (getuid() == 0) {
        rootEntries << QDir(QString(SYSTEM_USERS_DIR) + QString(DEFAULTUSERIGNITERSDIR)).entryList(QDir::Files);
    }
    auto allEntries = userEntries + standardEntries + rootEntries;
    allEntries.removeDuplicates();
    allEntries.sort();
    allEntries.replaceInStrings(QRegExp("\\.json"), "");
    return allEntries;
}


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
