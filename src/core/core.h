/*
    Author: Daniel (dmilith) Dettlaff
    © 2011-2014 - VerKnowSys
*/


#ifndef __CORE__
#define __CORE__


#include "../globals/globals.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <paths.h>

#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <libgen.h>
#include <getopt.h>
#include <termios.h>
#include <signal.h>
#include <dirent.h>

#ifdef __FreeBSD__
    #include <kvm.h>
    #include <sys/capability.h>
    #include <libprocstat.h>
    #include <libutil.h>
    #define ord(c) ((int)(unsigned char)(c))
#endif

#ifdef __APPLE__
    #include <mach/clock.h>
    #include <mach/clock_priv.h>
    #include <mach/clock_types.h>
    #include <sys/fcntl.h>
    #include <assert.h>
    #include <errno.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <sys/sysctl.h>
    #include <util.h>
    #include <sys/ioctl.h>
#endif

#ifdef __linux__
    #include <netinet/ip.h>
    #include <netinet/tcp.h>
    #include <netinet/udp.h>
    #include <net/if.h>
    #include <sys/param.h>
    #include <sys/fcntl.h>
    #include <sys/types.h>
    #define IPPROTO_DIVERT 254
#endif

#define NS 1000000000
#define DEFAULT_ALL_ALPHANUMERIC "0123456789.-+ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzżźłóćęąń"; // XXX: only Polish characters and ASCII for now.
#define DEFAULT_SHELL_COMMAND "/Software/Zsh/exports/zsh"

using namespace std;


class CORE {

    public:
        static int getOwner(char* path);
        static const char* getProcessUsage(int uid, bool consoleOutput = false);
        static const char* processDataToLearn(int uid);
        static std::string escapeJsonString(const std::string& input);
        static int adjustSystemTime(double offset);

    #ifdef __APPLE__
        static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount);
    #else
        static const char* procstat_files(struct procstat *procstat, struct kinfo_proc *kipp);
    #endif

};

#endif
