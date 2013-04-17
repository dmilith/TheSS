#ifndef __GLOBALS__
#define __GLOBALS__

    /* global constants */
    #define APP_VERSION "0.20.6"
    #define COPYRIGHT "Copyright © 2oo9-2o13 VerKnowSys.com - All Rights Reserved."
    #define MOTD_FILE "/etc/motd"

    /* default BSD case: */
    #define DEFAULT_SHELL_COMMAND "/Software/Zsh/exports/zsh" // "/bin/svdshell" //
    #define DEFAULT_SS_COMMAND "/bin/svdss"
    #define DEFAULT_JAVA_PATH "/Software/Openjdk7/openjdk7/"

    #ifdef __FreeBSD__
        #define DEFAULT_JAVA_BIN (DEFAULT_JAVA_PATH "bin/java")
    #endif

    // Darwin case:
    #ifdef __APPLE__
        #define CLOCK_REALTIME REALTIME_CLOCK
        // NOTE: Darwin uses same zsh path as BSD
        #define DEFAULT_JAVA_BIN "/usr/bin/java"
    #endif

    // Linux case:
    #ifdef __linux__
        #undef DEFAULT_SHELL_COMMAND
        #define DEFAULT_SHELL_COMMAND "/bin/zsh"
        #define DEFAULT_JAVA_BIN (DEFAULT_JAVA_PATH "bin/java")
    #endif


    #define SYSTEMUSERS_HOME_DIR "/SystemUsers"
    #define USERS_HOME_DIR "/Users"
    #define LIBRARIES_DIR "/lib"
    #define DEFAULT_BEHAVIORS_DIR "basesystem/behaviors"
    #define DEFAULT_BEHAVIORS_RAW "/output_raw_processes.raw.input"

    #define CORE_SVD_ID "boot"
    #define SOCK_FILE "svd.sock"
    #define LOCK_FILE "svd-core.lock"
    #define SOCKET_LOCK_FILE "svd-ss.lock"
    #define INTERNAL_LOG_FILE "svd-diagnostics.log"
    #define ROOT_JAR_FILE "/sbin/root.core"
    #define USER_JAR_FILE "/bin/user.core"

    #define DEFAULT_STRING_CODEC "utf8"
    #define LOCALE "en_GB.UTF-8"

    #define DEFAULT_USER_UID 501
    #define AMOUNT_OF_LOG_FILES_TO_KEEP 25

    #define DEFAULT_VPN_INTERFACE_SETUP_COMMAND "kldunload if_tap ; kldload if_tap && ifconfig tap0 create && ifconfig tap0 10.10.0.1 netmask 255.255.255.0"
    #define DEFAULT_VPN_INTERFACE_SHUTDOWN_COMMAND "ifconfig tap0 destroy"

    #define SOFTWARE_DIR "/Software"
    #define LOGS_DIR "/Logs"
    #define DEFAULT_PUBLIC_DIR "/Public"
    #define DEFAULT_USER_APPS_DIR "/Apps"
    #define DEFAULT_WEBAPPS_DIR "/WebApps"
    #define SOFTWARE_DATA_DIR "/SoftwareData"
    #define SYSTEM_USERS_DIR "/SystemUsers"
    #define DEFAULTSOFTWARETEMPLATEEXT ".json"
    #define DEFAULTSOFTWARETEMPLATE (USERS_HOME_DIR "/Common/Igniters/Default")
    #define DEFAULTSOFTWARETEMPLATESDIR (USERS_HOME_DIR "/Common/Igniters/Services")
    #define DEFAULTUSERIGNITERSDIR "/Igniters/Services"
    #define DEFAULT_SYSTEM_ADDRESS "127.0.0.1"
    #define DEFAULT_SERVICE_PID_FILE "/service.pid"
    #define DEFAULT_SERVICE_DOMAIN_FILE "/.domain"
    #define DEFAULT_SERVICE_PORTS_FILE "/.ports"
    #define DEFAULT_SERVICE_OUTPUT_FILE "/.output"
    #define DEFAULT_SERVICE_ERRORS_FILE "/.errors"
    #define DEFAULT_SERVICE_AUTOSTART_FILE "/.autostart"
    #define DEFAULT_SERVICE_RUNNING_FILE "/.running"
    #define DEFAULT_SERVICE_INSTALLING_FILE "/.installing"
    #define DEFAULT_SERVICE_AFTERSTOPPING_FILE "/.afterStopping"
    #define DEFAULT_SERVICE_AFTERSTARTING_FILE "/.afterStarting"
    #define DEFAULT_SERVICE_CONFIGURING_FILE "/.configuring"
    #define DEFAULT_SERVICE_RELOADING_FILE "/.reloading"
    #define DEFAULT_SERVICE_VALIDATING_FILE "/.validating"
    #define DEFAULT_SS_SHUTDOWN_HOOK_FILE "/.shutdown"

    #define ONE_SECOND_OF_DELAY 1000000 // 1 second
    #define DEFAULT_SERVICE_PAUSE_INTERVAL 5000000 // 5 seconds
    #define BABYSITTER_TIMEOUT_INTERVAL 30000000 // 00 seconds
    #define DEFAULT_GATHERING_PAUSE_MICROSECONDS 500000 // half a second
    #define DEFAULT_COUNT_OF_ROUNDS_OF_GATHERING 7200 // waiting half a second, hence 7200 is 60 minutes of gathering
    #define DEFAULT_USER_GROUP 0
    #define SOCK_DATA_PACKET_SIZE 32
    #define BUFFER_SIZE 256
    #define LOCK_FILE_OCCUPIED_ERROR 100
    #define CANNOT_LOCK_ERROR 101
    #define POPEN_ERROR 102
    #define CLASSPATH_DIR_MISSING_ERROR 103
    #define NOROOT_PRIVLEGES_ERROR 104
    #define SETUID_ERROR 105
    #define SETGID_ERROR 106
    #define FORK_ERROR 107
    #define EXEC_ERROR 108
    #define NO_UID_GIVEN_ERROR 109
    #define DIAGNOSTIC_LOG_ERROR 110
    #define AMBIGOUS_ENTRY_ERROR 111
    #define ROOT_UID_ERROR 112
    #define INSTALLATION_MISSING_ERROR 113
    #define STDIN_GETATTR_ERROR 114
    #define STDIN_SETATTR_ERROR 115
    #define STDIN_READ_ERROR 116
    #define STDOUT_WRITE_ERROR 117
    #define TERM_GETSIZE_ERROR 118
    #define TERM_GETATTR_ERROR 119
    #define PTY_WRITE_ERROR 120
    #define PTY_FORK_ERROR 121
    #define GETOPT_ERROR 122
    #define JSON_PARSE_ERROR 123
    #define NO_SUCH_FILE_ERROR 124
    #define NO_DEFAULT_IGNITERS_FOUND_ERROR 125
    #define JSON_FORMAT_EXCEPTION_ERROR 125
    #define OTHER_EXCEPTION_ERROR 126
    #define NO_NETWORK_ERROR 127

#endif
