#ifndef __GLOBALS__
#define __GLOBALS__

    /* global logger definitions */
    #define logTrace LOG_TRACE
    #define logDebug LOG_DEBUG
    #define logInfo  LOG_INFO
    #define logWarn  LOG_WARNING
    #define logError LOG_ERROR
    #define logFatal LOG_FATAL

    /* global constants */
    #define APP_VERSION "0.54.3"
    #define COPYRIGHT "Copyright © 2o13 VerKnowSys.com - All Rights Reserved."

    /* default BSD case: */
    #define DEFAULT_SHELL_COMMAND "/Software/Zsh/exports/zsh" // "/bin/svdshell" //

    #define DEFAULT_SS_COMMAND "/bin/svdss"

    #define SYSTEMUSERS_HOME_DIR "/SystemUsers"

    #define DEFAULT_BEHAVIORS_DIR "basesystem/behaviors"
    #define DEFAULT_BEHAVIORS_RAW "/output_raw_processes.raw.input"

    #define DEFAULT_STRING_CODEC "utf8"
    #define LOCALE "en_GB.UTF-8"

    #define NOTIFICATIONS_LAST_SHOWN 7 /* amount of notification before moving to history */
    #define NOTIFICATIONS_HISTORY_KEEP_UPTO 100 /* how many files to keep in history */

    #ifdef __APPLE__
        #define CLOCK_REALTIME REALTIME_CLOCK
        #define DEFAULT_DEATH_WATCHER_PAUSE 500000 /* .5 seconds by default for developer machine */
    #else
        #define DEFAULT_DEATH_WATCHER_PAUSE 4500000 /* 4.5 seconds by default for production */
    #endif

    #define DEFAULT_USER_UID 501
    #define AMOUNT_OF_LOG_FILES_TO_KEEP 125

    #define DEFAULT_VPN_INTERFACE_SETUP_COMMAND "kldunload if_tap ; kldload if_tap && ifconfig tap0 create &&  ifconfig tap1 create && ifconfig tap0 10.10.0.1 netmask 255.255.0.0 && ifconfig tap0 10.11.0.1 netmask 255.255.0.0 && echo 'success'"
    #define DEFAULT_VPN_INTERFACE_SETUP_COMMAND_LINUX "openvpn --mktun --dev tap0 && openvpn --mktun --dev tap1 && ifconfig tap0 10.20.0.1 netmask 255.255.0.0 && ifconfig tap1 10.21.0.1 netmask 255.255.0.0 && echo 'success'"
    #define DEFAULT_VPN_INTERFACE_SHUTDOWN_COMMAND "ifconfig tap0 destroy"

    #define SOFTWARE_DIR "/Software"
    #define LOGS_DIR "/Logs"
    #define DEFAULT_PUBLIC_DIR "/Public"
    #define DEFAULT_USER_APPS_DIR "/Apps"
    #define DEFAULT_WEBAPPS_DIR "/WebApps"
    #define SOFTWARE_DATA_DIR "/SoftwareData"
    #define SYSTEM_USERS_DIR "/SystemUsers"
    #define DEFAULTSOFTWARETEMPLATEEXT ".json"
    #define DEFAULTSOFTWARETEMPLATE "/Common/Igniters/Default"
    #define DEFAULTSOFTWARETEMPLATESDIR "/Common/Igniters/Services"
    #define DEFAULTUSERIGNITERSDIR "/Igniters/Services"
    #define DEFAULT_SYSTEM_ADDRESS "127.0.0.1"
    #define DEFAULT_WILDCARD_ADDRESS "0.0.0.0"
    #define DEFAULT_SYSTEM_DOMAIN "localhost"
    #define DEFAULT_SERVICE_PID_FILE "/service.pid"
    #define DEFAULT_SERVICE_DOMAIN_FILE "/.domain"
    #define DEFAULT_SERVICE_PORTS_DIR "/.ports/"
    #define NOTIFICATIONS_DATA_DIR "/.notifications/"
    #define NOTIFICATIONS_HISTORY_DATA_DIR "/.notifications-history/"
    #define DEFAULT_SERVICE_PORT_NUMBER "0"
    #define DEFAULT_SERVICE_DESTROY_FILE "/.destroy"
    #define DEFAULT_SERVICE_AUTOSTART_FILE "/.autostart"
    #define DEFAULT_SERVICE_RUNNING_FILE "/.running"
    #define DEFAULT_SERVICE_INSTALLING_FILE "/.installing"
    #define DEFAULT_SERVICE_AFTERSTOPPING_FILE "/.afterStopping"
    #define DEFAULT_SERVICE_AFTERSTARTING_FILE "/.afterStarting"
    #define DEFAULT_SERVICE_CONFIGURING_FILE "/.configuring"
    #define DEFAULT_SERVICE_RELOADING_FILE "/.reloading"
    #define DEFAULT_SERVICE_VALIDATING_FILE "/.validating"
    #define DEFAULT_SERVICE_VALIDATION_FAILURE_FILE "/.validationFailure"
    #define DEFAULT_SERVICE_LOG_FILE "/service.log"
    #define DEFAULT_SERVICE_OUTPUT_FILE DEFAULT_SERVICE_LOG_FILE
    #define DEFAULT_SS_SHUTDOWN_HOOK_FILE "/.shutdown"
    #define DEFAULT_SS_PROCESS_DATA_COLLECTION_HOOK_FILE "/.data-collector"
    #define DATA_COLLECTOR_SOCKET_FILE "/data-collector.sock"
    #define DEFAULT_SERVICE_CRON_WORKING_FILE "/.cron_working"
    #define NOTIFICATION_MANAGER_LOG_FILE "/.notifications-manager.log"
    #define CONTROL_PANEL_LOG_FILE "/.control-panel.log"
    #define DEFAULT_CRON_ENTRY "0 0/30 * * * ?"

    #define DEFAULT_PANEL_SCROLL_SIZE 256 /* panel log viewer scroll value in bytes */
    #define DEFAULT_PANEL_REFRESH_INTERVAL 250000

    #define ONE_SECOND_OF_DELAY 1000000 // 1 second
    #define DEFAULT_CRON_CHECK_DELAY 10000000 // 10s
    #define DEFAULT_SERVICE_PAUSE_INTERVAL 5000000 // 5 seconds
    #define BABYSITTER_TIMEOUT_INTERVAL 30000000 // 30 seconds
    #define COLLECTOR_TIMEOUT_INTERVAL 1000000 // 1 second
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
