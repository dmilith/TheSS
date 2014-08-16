/**
 *  @author dmilith, tallica
 *
 *   Shell wrapper with UID given as argument to main.
 *   This helper is used by SSHD side of ServeD
 *
 *   © 2011-2014 - VerKnowSys
 *
 */


#include "shellutils.h"


static struct termios saveTermios;
static int interactive;


string getUserHomeDirAndAskForName(int uid) {
    stringstream ss;
    string in = "";
    cout << endl << "Enter your user name: ";

    if (!getline(cin, in)) {
        if (cin.eof()) {
            cerr << "Terminated on EOF" << endl;
            exit(EXIT_SUCCESS);
        } else
            exit(EXIT_FAILURE);
    }

    // define allowed chars in user folder name entry:
    const char alphanum[] = DEFAULT_ALL_ALPHANUMERIC

    if (in.length() == 0) {
        cerr << "Empty user name given!" << endl;
        return getUserHomeDirAndAskForName(uid);
    }

    for (int i = 0; i < in.length(); i++) {
        if (strchr(alphanum, in.at(i)) == NULL) {
            #ifdef DEVEL
                cout << "Invalid characters given. You may only use these characters: '" << alphanum << "'" << endl;
            #endif
            return getUserHomeDirAndAskForName(uid);
        }
    }
    ss << DEFAULT_HOME_DIR << "/" << in;

    struct stat st;
    if (stat(ss.str().c_str(), &st) == 0) {
        cerr << "User with that name already exists!" << endl;
        return getUserHomeDirAndAskForName(uid);
    }

    return ss.str();
}


void ttySetRaw(void) {
    struct termios term;

    if (tcgetattr(STDIN_FILENO, &term) < 0) {
        cerr << "Unable to get terminal settings." << endl;
        exit(STDIN_GETATTR_ERROR);
    }

    saveTermios = term;

    /* Echo off, canonical mode off, extended input
       processing off, signal chars off. */
    term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    /* No SIGINT on BREAK, CR-to-NL off, input parity
       check off, don't strip 8th bit on input, output
       flow control off. */
    term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /* Clear size bits, parity checking off. */
    term.c_cflag &= ~(CSIZE | PARENB);

    /* Set 8 bits/char. */
    term.c_cflag |= CS8;

    /* Output processing off. */
    term.c_oflag &= ~(OPOST);

    /* Case B: 1 byte at a time, no timer. */
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) < 0) {
        cerr << "Unable to set terminal in raw mode." << endl;
        exit(STDIN_SETATTR_ERROR);
    }
}

void ttyReset(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &saveTermios) < 0)
        cerr << "Unable to reset terminal to its previous state." << endl;
}

void copyStreams(int master)
{
    pid_t child;
    pid_t parent = getpid();
    ssize_t nread;
    char buffer[BUFFER_SIZE];

    if ((child = fork()) < 0) {
        cerr << "Error forking child process!" << endl;
        exit(FORK_ERROR);
    } else if (child == 0) {
        /* Child process copies stdin to pty master. */
        for (;;) {
            if ((nread = read(STDIN_FILENO, buffer, BUFFER_SIZE)) < 0) {
                cerr << "Error reading from stdin!" << endl;
                exit(STDIN_READ_ERROR);
            } else if (nread == 0) /* EOF on stdin */
                break;
            if (write(master, buffer, nread) != nread) {
                cerr << "Error writing to PTY master!" << endl;
                exit(PTY_WRITE_ERROR);
            }
        }
        /* Send SIGTERM signal to parent process. */
        kill(parent, SIGTERM);
        /* Terminate child process. */
        exit(EXIT_SUCCESS);
    }

    /* Parent process copies pty master to stdout. */
    for (;;) {
        if ((nread = read(master, buffer, BUFFER_SIZE)) <= 0) /* Signal caught, error or EOF */
            break;
        if (write(STDOUT_FILENO, buffer, nread) != nread) {
            cerr << "Error writing to stdout!";
            exit(STDOUT_WRITE_ERROR);
        }
    }

    /* Send SIGTERM signal to child process. */
    kill(child, SIGTERM);
}


void execute(char **argv, int uid) {
    int master;
    pid_t pid;
    struct winsize size = {
        .ws_col = 80,
        .ws_row = 30
    };
    struct termios term;

    /* Get the current window size and settings of the terminal */
    if (interactive) {
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, &size) < 0) {
            cerr << "Can't get size of terminal window." << endl;
            exit(TERM_GETSIZE_ERROR);
        }
        if (tcgetattr(STDIN_FILENO, &term) < 0) {
            cerr << "Can't get terminal settings." << endl;
            exit(TERM_GETATTR_ERROR);
        }
        pid = forkpty(&master, NULL, &term, &size);
    } else
        pid = forkpty(&master, NULL, NULL, &size);

    if (pid < 0) {
        cerr << "Error forking PTY master process!" << endl;
        exit(PTY_FORK_ERROR);
    } else {
        stringstream hd, usr;

        if (uid == 0)
            hd << SYSTEM_USERS_DIR;
        else
            hd << DEFAULT_HOME_DIR << "/" << gatherUserNameFromDirEntry(uid, DEFAULT_HOME_DIR);

        usr << gatherUserNameFromDirEntry(uid, DEFAULT_HOME_DIR);;
        setenv("HOME", hd.str().c_str(), 1);
        setenv("~", hd.str().c_str(), 1);
        setenv("PWD", hd.str().c_str(), 1);
        setenv("OLDPWD", hd.str().c_str(), 1);
        setenv("USER", usr.str().c_str(), 1);
        setenv("LOGNAME", usr.str().c_str(), 1);
        setenv("LC_ALL", LOCALE, 1);
        setenv("LANG", LOCALE, 1);
        unsetenv("USERNAME");
        unsetenv("SUDO_USERNAME");
        unsetenv("SUDO_USER");
        unsetenv("SUDO_UID");
        unsetenv("SUDO_GID");
        unsetenv("SUDO_COMMAND");
        unsetenv("MAIL");
        char* dest = getenv("DESTPATH");
        if ((NULL != dest) && (strlen(dest) > 0)) {
            chdir(dest);
        } else
            chdir(hd.str().c_str());

        #ifdef DEVEL
            cerr << "Defined user basic env. Home dir: " << hd.str().c_str() << endl;
        #endif
        if (execvp(*argv, argv) < 0) {
            cerr << "Can't execute: " << *argv << endl;
            exit(EXEC_ERROR);
        }
    }

    if (interactive) {
        /* Set terminal in raw mode */
        ttySetRaw();
        /* Add "at exit" handler */
        atexit(ttyReset);
    }

    /* Copies between streams stdin, stdout and pty master */
    copyStreams(master);
}


static void printVersion(void) {
    cout << "ServeD Shell v" << APP_VERSION << " - " << COPYRIGHT << endl;
}


static void printUsage(void) {
    cout << endl;
    cout << "Usage: " << endl;
    cout << "  svdshell [option] [command]" << endl;
    cout << "  svdshell --uid=700 pstree" << endl;
    cout << "  svdshell -u700 -- ls -la" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "  -h, --help                This message." << endl;
    cout << "  -n, --non-interactive     Force non-interactive mode." << endl;
    cout << "  -u, --uid=<uid>           Spawn command with <uid> privileges." << endl;
    cout << "  -v, --version             Show copyright and version information." << endl;
}


int main(int argc, char *argv[]) {

    const char *defShell[] = {DEFAULT_SHELL_COMMAND, "-s", NULL};
    char **arguments = (char **) defShell;
    int opt = 0;

    printVersion();

    uid_t uid = getuid();
    gid_t gid = DEFAULT_USER_GROUP;

    interactive = isatty(STDIN_FILENO);

    /* Available options */
    static struct option options[] = {
        {"help", no_argument, 0, 'h'},
        {"non-interactive", no_argument, 0, 'n'},
        {"uid", optional_argument, 0, 'u'},
        {"version", no_argument, 0, 'v'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "hnu:v", options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printUsage();
                exit(EXIT_SUCCESS);
            case 'u': {
                    if (uid != 0) {
                        cerr << "You are not allowed to specify custom uid!" << endl;
                        exit(ROOT_UID_ERROR);
                    }

                    int optUid;
                    if (!optarg || !(istringstream(optarg) >> optUid) || optUid < 0) {
                        cerr << "Ambigous uid given!" << endl;
                        exit(AMBIGOUS_ENTRY_ERROR);
                    } else
                        uid = optUid;
                }
                break;
            case 'n':
                interactive = 0;
                break;
            case 'v':
                exit(EXIT_SUCCESS);
            case '?':
            case ':':
                exit(GETOPT_ERROR);
            default:
                break;
        }
    }

    /* Checking for additional arguments */
    if (optind < argc)
        arguments = argv + optind; /* Spawn custom command with uid privileges */

    /* Checking home directory existnace */
    struct stat st;
    string homeDir;

    if (uid == 0)
        homeDir = string(SYSTEM_USERS_DIR);
    else {
        const char *userName = gatherUserNameFromDirEntry(uid, DEFAULT_HOME_DIR);
        if (userName != NULL) {
            stringstream ss;
            ss << DEFAULT_HOME_DIR << "/" << userName;
            homeDir = ss.str();
        } else {
            /* No home directory, so let's ask a user for name, default domain and so on.. */
            homeDir = getUserHomeDirAndAskForName(uid);
        }
    }

    if (stat(homeDir.c_str(), &st) == 0) {
        #ifdef DEVEL
            cerr << "Home directory " << homeDir << " is present" << endl;
        #endif
    } else {
        #ifdef DEVEL
            cerr << "User home given: " << homeDir << endl;
            cerr << "Creating home directory " <<
                homeDir << " and chowning it for uid:" <<
                    uid << " and gid: " <<
                        gid << endl;
            mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #else
            mkdir(homeDir.c_str(), S_IRWXU | S_IXOTH); /* No rights for others than user - most safe */
        #endif
        chown(homeDir.c_str(), uid, gid);
    }

    /* setting user uid and gid privileges for shell */
    if (setuid(uid) != 0) {
        cerr << "Error setuid to uid: " << uid << endl;
        exit(SETUID_ERROR);
    }
    // #if !defined(__APPLE__) && !defined(__linux__)
    //     if (setgid(gid) != 0) {
    //         cerr << "Error setgid to gid: " << gid << endl;
    //         exit(SETGID_ERROR);
    //     }
    // #endif
    // chdir(homeDir.c_str());

    #ifdef DEVEL
        cerr << "Spawning command for uid: " << uid << ", gid: " << gid << endl;
        cerr << "Command line:";
        for (int i = 0; arguments[i] != NULL; i++)
            cerr << " " << arguments[i];
        cerr << endl;
        cerr << "Interactive mode: " << (interactive ? "yes" : "no") << endl;
    #endif

    execute(arguments, uid);
}
