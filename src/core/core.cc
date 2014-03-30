/*
    Author: Daniel (dmilith) Dettlaff, Michał (tallica) Lipski
    © 2011-2014 - VerKnowSys
*/

#include "core.h"


/**
 *  @author dmilith
 *
 *   This function is a temporary proxy to get owner of given file
 */
int getOwner(char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_uid;
    } else {
        return -1;
    }
}


/*
 *  @author tallica
 */
int adjustSystemTime(double offset) {
    #ifdef __APPLE__
        mach_timespec_t ts;
        clock_get_time(REALTIME_CLOCK, &ts);
    #else
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
    #endif

    long current_time  = (ts.tv_sec * NS) + ts.tv_nsec;
    long adjusted_time = current_time + (offset * NS);
    long nsec          = adjusted_time % NS;
    ts.tv_sec          = (adjusted_time - nsec) / NS;
    ts.tv_nsec         = nsec;

    #ifdef DEVEL
        cout << "Offset time: " << offset << " s" << endl;
        cout << "Current time: " << current_time << " ns; Adjusted time: " << adjusted_time << " ns" << endl;
    #endif

    #ifdef __APPLE__
        return ! clock_set_time(REALTIME_CLOCK, ts);
    #else
        return ! clock_settime(CLOCK_REALTIME, &ts);
    #endif
}


// extern std::string escapeJsonString(const std::string& input);


#ifdef __APPLE__


typedef struct kinfo_proc kinfo_proc;


int GetBSDProcessList(kinfo_proc **procList, size_t *procCount) {
    int                 err;
    kinfo_proc *        result;
    bool                done;
    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t              length;

    assert( procList != NULL);
    // assert(*procList == NULL);
    assert(procCount != NULL);

    *procCount = 0;
    result = NULL;
    done = false;
    do {
        assert(result == NULL);

        // Call sysctl with a NULL buffer.

        length = 0;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                      NULL, &length,
                      NULL, 0);
        if (err == -1) {
            err = errno;
        }

        // Allocate an appropriately sized buffer based on the results
        // from the previous call.
        if (err == 0) {
            result = (kinfo_proc*) malloc(length);
            if (result == NULL) {
                err = ENOMEM;
            }
        }

        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.
        if (err == 0) {
            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                          result, &length,
                          NULL, 0);
            if (err == -1) {
                err = errno;
            }
            if (err == 0) {
                done = true;
            } else if (err == ENOMEM) {
                assert(result != NULL);
                free(result);
                result = NULL;
                err = 0;
            }
        }
    } while (err == 0 && ! done);

    // Clean up and establish post conditions.
    if (err != 0 && result != NULL) {
        free(result);
        result = NULL;
    }
    *procList = result;
    if (err == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }
    assert( (err == 0) == (*procList != NULL) );
    return err;
}


const char* processDataToLearn(int uid) {

    string output;
    // const int pagesize = getpagesize();
    size_t count = 0;
    kinfo_proc *result = NULL;
    kinfo_proc *procs = NULL;
    result = (kinfo_proc*)malloc(sizeof(kinfo_proc));

    if (GetBSDProcessList(&result, &count) == 0) {
        for (int i = 0; i < count; ++i) {

            procs = &result[i];

            stringstream out;
            out << (procs->kp_proc.p_comm) << " "
                << (procs->kp_proc.p_pid) << " "
                << (procs->kp_proc.p_rtime.tv_sec) << " "
                << (procs->kp_proc.p_cpticks) << " "
                << count << " "
                << endl;

            output += out.str();
        }
    }
    free(result);
    return output.c_str();
}


const char* getProcessUsage(int uid, bool consoleOutput) {
    stringstream out;
    mach_timespec_t ts;
    clock_get_time(REALTIME_CLOCK, &ts);
    out << "{\"time\": \"" << ts.tv_sec << "\"}";
    return out.str().c_str();
}

#else


#define ord(c) ((int)(unsigned char)(c))


const char* addr_to_string(struct sockaddr_storage *ss) {
    char buffer2[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *sin6;
    struct sockaddr_in *sin;
    struct sockaddr_un *sun;
    stringstream out;
    switch (ss->ss_family) {
        case AF_LOCAL:
            sun = (struct sockaddr_un *)ss;
            out << sun->sun_path;
            break;
        case AF_INET:
            sin = (struct sockaddr_in *)ss;
            out << inet_ntoa(sin->sin_addr) << ":" << ntohs(sin->sin_port);
            break;
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *)ss;
            if (inet_ntop(AF_INET6, &sin6->sin6_addr, buffer2, sizeof(buffer2)) != NULL)
                out << buffer2 << "." << ntohs(sin6->sin6_port);
            break;
        default:
            out << "*:*";
            break;
    }
    return out.str().c_str();
}


static const char* protocol_to_string(int domain, int type, int protocol) {
    switch (domain) {
        case AF_INET:
        case AF_INET6:
            switch (protocol) {
            case IPPROTO_TCP:
                return ("TCP");
            case IPPROTO_UDP:
                return ("UDP");
            case IPPROTO_ICMP:
                return ("ICM");
            case IPPROTO_RAW:
                return ("RAW");
            case IPPROTO_SCTP:
                return ("SCT");
            case IPPROTO_DIVERT:
                return ("IPD");
            default:
                return ("IP?");
            }
        case AF_LOCAL:
            switch (type) {
            case SOCK_STREAM:
                return ("UDS");
            case SOCK_DGRAM:
                return ("UDD");
            default:
                return ("UD?");
            }
        default:
            return ("?");
    }
}


const char* procstat_files(struct procstat *procstat, struct kinfo_proc *kipp) {
    struct sockstat sock;
    struct filestat_list *head;
    struct filestat *fst;
    stringstream out;

    head = procstat_getfiles(procstat, kipp, 0);
    if (head == NULL)
        return "ERROR: HEAD NULL";

    STAILQ_FOREACH(fst, head, next) {
        if (fst->fs_type == PS_FST_TYPE_SOCKET) { // only sockets
            if (procstat_get_socket_info(procstat, fst, &sock, NULL) != 0) {
                return "ERROR PROCSTAT GET SOCKET";
                break;
            }
            if ((sock.dom_family == AF_INET) || (sock.dom_family == AF_INET6)) { // only INET domain
                out << protocol_to_string(sock.dom_family, sock.type, sock.proto) << " ";
                if (sock.dom_family == AF_LOCAL) {
                    struct sockaddr_un *sun = (struct sockaddr_un *)&sock.sa_local;
                    if (sun->sun_path[0] != 0)
                        out << addr_to_string(&sock.sa_local);
                    else
                        out << addr_to_string(&sock.sa_peer);
                } else {
                    out << addr_to_string(&sock.sa_local) << "-" << addr_to_string(&sock.sa_peer);
                }
                out << "#";
            }
        }
    }
    procstat_freefiles(procstat, head);
    return out.str().c_str();
}


const char* getProcessUsage(int uid, bool consoleOutput) {

    int count = 0;
    char** args = NULL;
    string command, output;
    int pagesize = getpagesize();

    kvm_t* kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
    if (kd == 0) {
        if (consoleOutput)
            cerr << "Error initializing kernel descriptor!" << endl;
        return (char*)"KDERR";
    }

    kinfo_proc* procs = kvm_getprocs(kd, KERN_PROC_UID, uid, &count); // get processes directly from BSD kernel
    if (count <= 0) {
        if (consoleOutput)
            cerr << "No processes for given UID!" << endl;
        return (char*)"NOPCS";
    }

    if (consoleOutput) {
        cout << "Process count: " << count << ". Owner UID: " << uid << endl;
        cout << setiosflags(ios::left)
            << setw(6) << "| NO:"
            << setw(27) << "| NAME:"
            << setw(52) << "| CMD:"
            << setw(10) << "| PID:"
            << setw(10) << "| PPID:"
            << setw(10) << "| RSS:"
            << setw(10) << "| MRSS:"
            << setw(16) << "| RUN-TIME(ms):"
            << setw(12) << "| BLK-IN:"
            << setw(12) << "| BLK-OUT:"
            << setw(6) << "| THR:"
            << setw(6) << "| PRI-NRML:"
            << endl;
    }

    for (int i = 0; i < count; ++i) {
        stringstream out;
        command = "";
        args = kvm_getargv(kd, procs, 0);
        for (int y = 0; (args != 0) && (args[y] != 0); y++)
            if (y == 0)
                command = string(args[y]);
            else
                command += " " + string(args[y]);

        unsigned int cnt = 0;
        struct procstat* procstat = procstat_open_sysctl();
        struct kinfo_proc *kproc = procstat_getprocs(procstat, KERN_PROC_PID, procs->ki_pid, &cnt);
        string netinfo = "";
        if (cnt != 0)
            netinfo = procstat_files(procstat, kproc);
        procstat_freeprocs(procstat, kproc);
        procstat_close(procstat);
        procstat = NULL;
        kproc = NULL;

        if (consoleOutput) {
            out << setiosflags(ios::left)
                << "| " << setw(4) << (i + 1)
                << "| " << setw(25) << (procs->ki_comm)
                << "| " << setw(50) << (command)
                << "| " << setw(8) << (procs->ki_pid)
                << "| " << setw(8) << (procs->ki_ppid)
                << "| " << setw(8) << (procs->ki_rssize * pagesize)
                << "| " << setw(8) << (procs->ki_rusage.ru_maxrss * pagesize)
                << "| " << setw(14) << (procs->ki_runtime / 1000) // miliseconds
                << "| " << setw(10) << (procs->ki_rusage.ru_inblock)
                << "| " << setw(10) << (procs->ki_rusage.ru_oublock)
                << "| " << setw(4) << (procs->ki_numthreads)
                << "| " << setw(6) << ord(procs->ki_pri.pri_level)
                << "| " << netinfo
                << endl;
        } else {
            if (i == 0) {
                out << "[";
            }
            out << "{\"pid\":" << (procs->ki_pid) << ","
                << "\"ppid\":" << (procs->ki_ppid) << ","
                << "\"name\":\"" << escapeJsonString(procs->ki_comm) << "\","
                << "\"cmd\":\"" << escapeJsonString(command) << "\","
                << "\"rss\":" << (procs->ki_rssize * pagesize) << ","
                << "\"mrss\":" << (procs->ki_rusage.ru_maxrss * pagesize) << ","
                << "\"runtime\":" << (procs->ki_runtime / 1000) << ","
                << "\"blk-in\":" << (procs->ki_rusage.ru_inblock) << ","
                << "\"blk-out\":" << (procs->ki_rusage.ru_oublock) << ","
                << "\"thr\":" << (procs->ki_numthreads) << ","
                << "\"pri-nrml\":" << ord(procs->ki_pri.pri_level) << ","
                << "\"netinfo\":\"" << escapeJsonString(netinfo) << "\"}";

            if (i == count - 1) {
                out << "]";
            } else {
                out << ",";
            }
        }

        args = NULL;
        output += out.str();
        procs++;
    }

    kvm_close(kd);
    return output.c_str();
}


const char* processDataToLearn(int uid) {

    int count = 0;
    char** args = NULL;
    string output;
    const int pagesize = getpagesize();
    const int totalMem = pagesize * sysconf(_SC_PHYS_PAGES);

    kvm_t* kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
    if (kd == 0) {
        return (char*)"{\"message\": \"KDERR\", \"status\":-1}";
    }

    kinfo_proc* procs = kvm_getprocs(kd, KERN_PROC_UID, uid, &count); // get processes directly from BSD kernel
    if (count <= 0) {
        return (char*)"{\"message\": \"NOPCS\", \"status\":-2}";
    }

    output += "{\"message\": \"Ok\", \"status\":0, \"content\": [";
    for (int i = 0; i < count; ++i) {
        stringstream out;
        args = kvm_getargv(kd, procs, 0);
        out << "{\"cmd\":\"" << (procs->ki_comm) << "\","
            << "\"pid\":" << (procs->ki_pid) << ","
            << "\"ppid\":" << (procs->ki_ppid) << ","
            << "\"runt\":" << (procs->ki_runtime / 1000) << ","
            << "\"ioin\":" << (procs->ki_rusage.ru_inblock) << ","
            << "\"ioout\":" << (procs->ki_rusage.ru_oublock) << ","
            << "\"rss\":" << (procs->ki_rssize * pagesize) << "}";
        if (i + 1 != count) out << ","; // if last element not detected add a comma
        args = NULL;
        output += out.str();
        procs++;
    }

    kvm_close(kd);
    output += "]}";
    return output.c_str();
}

#endif


