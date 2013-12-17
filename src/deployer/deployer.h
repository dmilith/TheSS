/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __DEPLOYER_H__
#define __DEPLOYER_H__


#include "../service_spawner/utils.h"
#include "../service_spawner/process.h"
#include "../globals/globals.h"
#include "webapp_types.h"

#include <QObject>


#define MAX_DEPLOYS_TO_KEEP 5 /* keep 5 deploys in releases */
#define MINIMUM_DISK_SPACE_IN_MEGS 512


inline QString nginxEntry(WebAppTypes type, QString latestReleaseDir, QString domain, QString serviceName, QString stage, QString port) {
    switch (type) {
        case StaticSite:
            return "\n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    access_log off; \n\
    location / { \n\
        index index.html index.htm; \n\
        expires 30d; \n\
    } \n\
}\n";

        case RubySite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server 127.0.0.1:" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header Host $http_host; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
    } \n\
} \n";


        case NodeSite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server 127.0.0.1:" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header Host $http_host; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
    } \n\
} \n";


        case PhpSite: {
            QString prefix = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + serviceName;
writeToFile(prefix + "/service.conf", "[global] \n\
    error_log = " + prefix + "/service.log \n\
    ;emergency_restart_threshold = 0 \n\
    ;emergency_restart_interval = 0 \n\
    ;process_control_timeout = 0 \n\
    ; process.max = 128 \n\
    ;daemonize = yes \n\
    ;rlimit_files = 1024 \n\
    ;rlimit_core = 0 \n\
    events.mechanism = kqueue \n\
    ;security.limit_extensions = .php .html .htm .less .js .coffee \n\
[www] \n\
    listen = 127.0.0.1:" + port + " \n\
    pm = dynamic \n\
    pm.max_children = 5 \n\
    pm.start_servers = 2 \n\
    pm.min_spare_servers = 1 \n\
    pm.max_spare_servers = 3 \n\
    ;pm.process_idle_timeout = 10s; \n\
    ;pm.max_requests = 500 \n");

writeToFile(prefix + "/service.ini", "[PHP] \n\
    engine = On \n\
    short_open_tag = Off \n\
    asp_tags = Off \n\
    precision = 14 \n\
    y2k_compliance = On \n\
    output_buffering = 4096 \n\
    zlib.output_compression = Off \n\
    implicit_flush = Off \n\
    unserialize_callback_func = \n\
    serialize_precision = 100 \n\
    allow_call_time_pass_reference = Off \n\
    safe_mode = Off \n\
    safe_mode_gid = Off \n\
    safe_mode_include_dir = \n\
    safe_mode_exec_dir = \n\
    safe_mode_allowed_env_vars = PHP_ \n\
    safe_mode_protected_env_vars = LD_LIBRARY_PATH \n\
    open_basedir = " + latestReleaseDir + " \n\
    disable_functions = exec,popen,system \n\
    disable_classes = \n\
    realpath_cache_size = 128k \n\
    expose_php = Off \n\
    max_execution_time = 30 \n\
    max_input_time = 60 \n\
    memory_limit = 512M \n\
    error_reporting = E_ALL & ~E_DEPRECATED \n\
    display_errors = On \n\
    display_startup_errors = Off \n\
    log_errors = On \n\
    log_errors_max_len = 1024 \n\
    ignore_repeated_errors = Off \n\
    ignore_repeated_source = Off \n\
    report_memleaks = On \n\
    track_errors = Off \n\
    html_errors = Off \n\
    variables_order = \"GPCS\" \n\
    request_order = \"GP\" \n\
    register_globals = Off \n\
    register_long_arrays = Off \n\
    register_argc_argv = Off \n\
    auto_globals_jit = On \n\
    post_max_size = 8M \n\
    magic_quotes_gpc = On \n\
    magic_quotes_runtime = Off \n\
    magic_quotes_sybase = Off \n\
    auto_prepend_file = \n\
    auto_append_file = \n\
    default_mimetype = \"text/html\" \n\
    default_charset = \"UTF-8\" \n\
    doc_root = \n\
    user_dir = " + latestReleaseDir + " \n\
    enable_dl = Off \n\
    file_uploads = On \n\
    upload_max_filesize = 50M \n\
    max_file_uploads = 20 \n\
    allow_url_fopen = On \n\
    allow_url_include = Off \n\
    default_socket_timeout = 60 \n\
[Pdo_mysql] \n\
    pdo_mysql.cache_size = 2000 \n\
    pdo_mysql.default_socket= \n\
[Syslog] \n\
    define_syslog_variables  = Off \n\
[mail function] \n\
    SMTP = localhost \n\
    smtp_port = 25 \n\
    mail.add_x_header = On \n\
[SQL] \n\
    sql.safe_mode = Off \n\
[ODBC] \n\
    odbc.allow_persistent = On \n\
    odbc.check_persistent = On \n\
    odbc.max_persistent = -1 \n\
    odbc.max_links = -1 \n\
    odbc.defaultlrl = 4096 \n\
    odbc.defaultbinmode = 1 \n\
[Interbase] \n\
    ibase.allow_persistent = 1 \n\
    ibase.max_persistent = -1 \n\
    ibase.max_links = -1 \n\
[MySQL] \n\
    mysql.allow_local_infile = On \n\
    mysql.allow_persistent = On \n\
    mysql.cache_size = 2000 \n\
    mysql.max_persistent = -1 \n\
    mysql.max_links = -1 \n\
    mysql.default_port = \n\
    mysql.default_socket = \n\
    mysql.default_host = \n\
    mysql.default_user = \n\
    mysql.default_password = \n\
    mysql.connect_timeout = 60 \n\
    mysql.trace_mode = Off \n\
[MySQLi] \n\
    mysqli.max_persistent = -1 \n\
    mysqli.allow_persistent = On \n\
    mysqli.max_links = -1 \n\
    mysqli.cache_size = 2000 \n\
    mysqli.default_port = 3306 \n\
    mysqli.default_socket = \n\
    mysqli.default_host = \n\
    mysqli.default_user = \n\
    mysqli.default_pw = \n\
    mysqli.reconnect = Off \n\
[mysqlnd] \n\
    mysqlnd.collect_statistics = On \n\
    mysqlnd.collect_memory_statistics = Off \n\
[OCI8] \n\
    pgsql.allow_persistent = On \n\
    pgsql.auto_reset_persistent = Off \n\
    pgsql.max_persistent = -1 \n\
    pgsql.max_links = -1 \n\
    pgsql.ignore_notice = 0 \n\
    pgsql.log_notice = 0 \n\
    sybct.allow_persistent = On \n\
    sybct.max_persistent = -1 \n\
    sybct.max_links = -1 \n\
    sybct.min_server_severity = 10 \n\
    sybct.min_client_severity = 10 \n\
    bcmath.scale = 0 \n\
    session.save_handler = files \n\
    session.use_cookies = 1 \n\
    session.use_only_cookies = 1 \n\
    session.name = PHPSESSID \n\
    session.auto_start = 0 \n\
    session.cookie_lifetime = 0 \n\
    session.cookie_path = / \n\
    session.cookie_domain = \n\
    session.cookie_httponly = \n\
    session.serialize_handler = php \n\
    session.gc_probability = 1 \n\
    session.gc_divisor = 1000 \n\
    session.gc_maxlifetime = 1440 \n\
    session.bug_compat_42 = Off \n\
    session.bug_compat_warn = Off \n\
    session.referer_check = \n\
    session.entropy_length = 0 \n\
    session.cache_limiter = nocache \n\
    session.cache_expire = 180 \n\
    session.use_trans_sid = 0 \n\
    session.hash_function = 0 \n\
    session.hash_bits_per_character = 5 \n\
    url_rewriter.tags = \"a=href,area=href,frame=src,input=src,form=fakeentry\" \n\
[MSSQL] \n\
    mssql.allow_persistent = On \n\
    mssql.max_persistent = -1 \n\
    mssql.max_links = -1 \n\
    mssql.min_error_severity = 10 \n\
    mssql.min_message_severity = 10 \n\
    mssql.compatability_mode = Off \n\
    mssql.secure_connection = Off \n\
[Tidy] \n\
    tidy.clean_output = Off \n\
[soap] \n\
    soap.wsdl_cache_enabled=1 \n\
    soap.wsdl_cache_dir=\"" + latestReleaseDir + "/tmp\" \n\
    soap.wsdl_cache_ttl=86400 \n\
    soap.wsdl_cache_limit = 5 \n\
[ldap] \n\
    ldap.max_links = -1 \n\
[dba] \n\
    apc.include_once_override = Off \n\
    apc.canonicalize = On");

writeToFile(latestReleaseDir + "/fastcgi_params",
"fastcgi_param  QUERY_STRING       $query_string; \n\
fastcgi_param  REQUEST_METHOD     $request_method; \n\
fastcgi_param  CONTENT_TYPE       $content_type; \n\
fastcgi_param  CONTENT_LENGTH     $content_length; \n\
fastcgi_param  SCRIPT_NAME        $fastcgi_script_name; \n\
fastcgi_param  REQUEST_URI        $request_uri; \n\
fastcgi_param  DOCUMENT_URI       $document_uri; \n\
fastcgi_param  DOCUMENT_ROOT      $document_root; \n\
fastcgi_param  SERVER_PROTOCOL    $server_protocol; \n\
fastcgi_param  GATEWAY_INTERFACE  CGI/1.1; \n\
fastcgi_param  SERVER_SOFTWARE    nginx/$nginx_version; \n\
fastcgi_param  REMOTE_ADDR        $remote_addr; \n\
fastcgi_param  REMOTE_PORT        $remote_port; \n\
fastcgi_param  SERVER_ADDR        $server_addr; \n\
fastcgi_param  SERVER_PORT        $server_port; \n\
fastcgi_param  SERVER_NAME        $server_name; \n");
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server 127.0.0.1:" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    location ~* ^.+.(css|jpg|jpeg|gif|png|js|ico|xml|mp3|eps|cdr|rar|zip|p7|pdf|swf)$ { \n\
        access_log off; \n\
        root " + latestReleaseDir + "; \n\
        add_header Cache-Control max-age=315360000; \n\
        expires 30d; \n\
        break; \n\
    } \n\
    location / { \n\
        root " + latestReleaseDir + "; \n\
        index index.php; \n\
        if (-f $request_filename) { \n\
            break; \n\
        } \n\
        include " + latestReleaseDir + "/fastcgi_params; \n\
        fastcgi_param SCRIPT_FILENAME " + latestReleaseDir + "$fastcgi_script_name; \n\
        fastcgi_param PATH_INFO $fastcgi_script_name; \n\
        fastcgi_pass " + serviceName + "-" + stage + "; \n\
    } \n\
    location ~ /\\. { \n\
        deny  all; \n\
    } \n\
}\n";
        }

        case NoType:
            return "";

    }
}


inline QString databaseYmlEntry(WebDatabase db, QString stage, QString databaseName) {
    switch (db) {

        case Postgresql:
            return stage + ": \n\
  adapter: postgresql \n\
  encoding: unicode \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  pool: 5 \n\
  port: <%= File.read(ENV['HOME'] + \"/SoftwareData/Postgresql/.ports/0\") %> \n\
  host: <%= ENV['HOME'] + \"/SoftwareData/Postgresql/\" %> \n\
"; // XXX: should contains latestRelease cause of potential database failure that might happen after db:migrate

        case Mysql:
            return ""; // NOTE: NYI

        case Mongo:
            return ""; // NOTE: NYI

        case Redis:
            return ""; // NOTE: NYI

        case ElasticSearch:
            return ""; // NOTE: NYI

        case Sphinx:
            return ""; // NOTE: NYI

        case NoDB: /* NoDB means we might want to use SQLite3 driver */
            return "\n\
development: \n\
  adapter: sqlite3 \n\
  database: db/db_" + databaseName + "_" + stage + ".sqlite3 \n\
  timeout: 5000 \n\
";

    }
}


QString getDbName(WebDatabase db) {
    switch (db) {
        case Postgresql: return "Postgresql";
        case Mysql: return "Mysql";
        case Mongo: return "Mongo";
        case Redis: return "Redis";
        case ElasticSearch: return "ElasticSearch";
        case Sphinx: return "Sphinx";
        case NoDB: return "NoDB";
    }
}

void installDependencies();
void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch, QString& stage);
void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch);
QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain);
QString buildEnv(QString& serviceName, QStringList deps);

#endif
