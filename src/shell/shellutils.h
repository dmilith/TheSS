
#ifndef __SHELL_UTILS_H__
#define __SHELL_UTILS_H__


#include "../core/core.h"

#ifdef __linux__
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <pty.h>
    #include <utmp.h>
    #include <pty.h>     /* for openpty and forkpty */
#endif

const char* gatherUserNameFromDirEntry(int uid, const char* users_home_dir);


#endif
