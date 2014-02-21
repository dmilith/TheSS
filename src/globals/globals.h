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
    #define APP_VERSION "0.80.5"
    #define COPYRIGHT "(c) 2o13-2o14 verknowsys.com"

    /* default BSD case: */
    #define DEFAULT_SHELL_COMMAND "/Software/Zsh/exports/zsh" // "/bin/svdshell" //

    #define DEFAULT_BEHAVIORS_DIR "basesystem/behaviors"
    #define DEFAULT_BEHAVIORS_RAW "/output_raw_processes.raw.input"

    #define DEFAULT_STRING_CODEC "utf8"
    #define LOCALE "en_GB.UTF-8"

    #define NOTIFICATIONS_OPTIONS_BAR_HEIGHT 2
    #define NOTIFICATIONS_LAST_SHOWN 11 /* amount of notification before moving to history */
    #define NOTIFICATIONS_HISTORY_KEEP_UPTO 100 /* how many files to keep in history */

    #ifdef __APPLE__
        #define CLOCK_REALTIME REALTIME_CLOCK
        #define DEFAULT_DEATH_WATCHER_PAUSE 500000 /* .5 seconds by default for developer machine */
    #else
        #define DEFAULT_DEATH_WATCHER_PAUSE 2000000 /* 2 seconds by default for production */
    #endif

    #define DEFAULT_USER_UID 501
    #define AMOUNT_OF_LOG_FILES_TO_KEEP 125

    #define DEFAULT_VPN_INTERFACE_SETUP_COMMAND "kldunload if_tap ; kldload if_tap && ifconfig tap0 create &&  ifconfig tap1 create && ifconfig tap0 10.10.0.1 netmask 255.255.0.0"
    #define DEFAULT_VPN_INTERFACE_SHUTDOWN_COMMAND "ifconfig tap0 destroy"

    #define SOFTWARE_DIR "/Software/"
    #define LOGS_DIR "/Logs"
    #define DEFAULT_PROXY_FILE "/proxy.conf"
    #define DEFAULT_APP_PROXY_FILE "/.proxy.conf" /* addition to proxy from app repo */
    #define DEFAULT_PUBLIC_DIR "/Public/"
    #define DEFAULT_HOME_DIR "/Users/"
    #define DEFAULT_USER_APPS_DIR "/Apps"
    #define DEFAULT_WEBAPPS_DIR "/WebApps"
    #define WEB_APP_PID_FILE_POSTFIX "-prev"
    #define WEB_APP_PID_FILE_POSTFIX_NEXT "-next"
    #define SOFTWARE_DATA_DIR "/SoftwareData/"
    #define SYSTEM_USERS_DIR "/SystemUsers/"
    #define DEFAULT_GIT_REPOSITORY_DIR "/Repos/"
    #define DEFAULT_SOFTWARE_TEMPLATE_EXT ".json"
    #define DEFAULT_SERVICE_INSTALLED_EXT ".installed"
    #define DEFAULT_SOFTWARE_TEMPLATE (SYSTEM_USERS_DIR "Igniters/Default")
    #define DEFAULT_SOFTWARE_TEMPLATES_DIR (SYSTEM_USERS_DIR "Igniters/Services")
    #define DEFAULT_USER_IGNITERS_DIR "/Igniters/Services"
    #define DEFAULT_LOCAL_ADDRESS "127.0.0.1"
    #define DEFAULT_WILDCARD_ADDRESS "0.0.0.0"
    #define DEFAULT_SYSTEM_DOMAIN "localhost"

    #define DEFAULT_SERVICE_SOCKET_FILE "/service.sock"
    #define DEFAULT_SERVICE_PID_FILE "/service.pid"
    #define DEFAULT_SERVICE_ENV_FILE "/service.env"
    #define DEFAULT_SERVICE_LOG_FILE "/service.log"
    #define DEFAULT_SERVICE_CONF_FILE "/service.conf"
    #define DEFAULT_SERVICE_DEPENDENCIES_FILE "/.dependencies"

    /* triggers */
    #define INSTALL_TRIGGER_FILE "/.install"
    #define CONFIGURE_TRIGGER_FILE "/.configure"
    #define RECONFIGURE_TRIGGER_FILE "/.reconfigure"
    #define RECONFIGURE_WITHOUT_DEPS_TRIGGER_FILE "/.reconfigureWithoutDeps"
    #define START_TRIGGER_FILE "/.start"
    #define START_WITHOUT_DEPS_TRIGGER_FILE "/.startWithoutDeps"
    #define STOP_TRIGGER_FILE "/.stop"
    #define STOP_WITHOUT_DEPS_TRIGGER_FILE "/.stopWithoutDeps"
    #define AFTERSTART_TRIGGER_FILE "/.afterStart"
    #define AFTERSTOP_TRIGGER_FILE "/.afterStop"
    #define RESTART_TRIGGER_FILE "/.restart"
    #define RESTART_WITHOUT_DEPS_TRIGGER_FILE "/.restartWithoutDeps"
    #define RELOAD_TRIGGER_FILE "/.reload"
    #define VALIDATE_TRIGGER_FILE "/.validate"
    #define AUTOSTART_TRIGGER_FILE "/.autostart"

    /* states */
    #define DEFAULT_SERVICE_RUNNING_FILE "/.running"
    #define DEFAULT_SERVICE_INSTALLING_FILE "/.installing"
    #define DEFAULT_SERVICE_AFTERSTOPPING_FILE "/.afterStopping"
    #define DEFAULT_SERVICE_AFTERSTARTING_FILE "/.afterStarting"
    #define DEFAULT_SERVICE_CONFIGURING_FILE "/.configuring"
    #define DEFAULT_SERVICE_RELOADING_FILE "/.reloading"
    #define DEFAULT_SERVICE_VALIDATING_FILE "/.validating"
    #define DEFAULT_SERVICE_DEPLOYING_FILE "/.deploying"
    #define DEFAULT_SERVICE_CRON_WORKING_FILE "/.cronWorking"

    #define DEFAULT_SERVICE_CONFIGURED_FILE "/.configured"
    #define DEFAULT_SERVICE_VALIDATION_FAILURE_FILE "/.validationFailure"
    #define DEFAULT_SERVICE_LATEST_RELEASE_FILE "/.service-current-release"
    #define DEFAULT_SERVICE_PREVIOUS_RELEASE_FILE "/.service-previous-release"
    #define DEFAULT_SERVICE_RELEASES_HISTORY "/.service-releases"
    #define NOTIFICATIONS_HISTORY_DATA_DIR "/.notifications-history/"

    #define DEFAULT_STANDALONE_DIR "/standalone/"
    #define DEFAULT_RELEASES_DIR "/releases/"
    #define DEFAULT_SHARED_DIR "/shared/"
    #define DEFAULT_SERVICE_DOMAINS_DIR "/.domains/"
    #define DEFAULT_SERVICE_LOGS_DIR "/.logs/"
    #define DEFAULT_SERVICE_PORTS_DIR "/.ports/"
    #define DEFAULT_SERVICE_ENVS_DIR "/.envs/"
    #define DEFAULT_SERVICE_PIDS_DIR "/.pids/"
    #define DEFAULT_SERVICE_CONFS_DIR "/.confs/"
    #define DEFAULT_SERVICE_SSLS_DIR "/.ssls/"

    #define DEFAULT_CA_CERT_ROOT_SITE "http://curl.haxx.se/ca"
    #define DEFAULT_SSL_CA_FILE "/cacert.pem"

    /* global definitions */
    #define DATA_COLLECTOR_SOCKET_FILE "/data-collector.sock"
    #define NOTIFICATIONS_DATA_DIR "/.notifications/"
    #define NOTIFICATION_MANAGER_LOG_FILE "/.notifications-manager.log"
    #define DEFAULT_SERVICE_PORT_NUMBER "0"
    #define DEFAULT_SERVICE_OUTPUT_FILE (DEFAULT_SERVICE_LOG_FILE "-stderr.log")
    #define DEFAULT_SS_SHUTDOWN_FILE ".shutdown"
    #define DEFAULT_SS_SHUTDOWN_HOOK_FILE ("/" DEFAULT_SS_SHUTDOWN_FILE)
    #define DEFAULT_SS_GRACEFUL_SHUTDOWN_FILE ".shutdownGracefully"
    #define DEFAULT_SS_GRACEFUL_SHUTDOWN_HOOK_FILE ("/" DEFAULT_SS_GRACEFUL_SHUTDOWN_FILE)
    #define DEFAULT_SS_PROCESS_DATA_COLLECTION_HOOK_FILE "/.data-collector"
    #define DEFAULT_SS_LOG_FILE "/.ss.log"
    #define CONTROL_PANEL_LOG_FILE "/.control-panel.log"
    #define DEFAULT_CRON_ENTRY "0 4 * * * ?" /* exactly 4am */

    #define ONE_SECOND_OF_DELAY 1000000 // 1 second
    #define DEFAULT_CRON_CHECK_DELAY 60000000 // 60s
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
