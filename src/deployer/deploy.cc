/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "deploy.h"


const QStringList getStandaloneDeps() {
    QStringList output; /* XXX: it's hacked hack, but I still have no better solution for this problem.. */
    output << "postgresql" << "mysql" << "redis" << "redis-usock" << "nginx" << "passenger" << "sphinx" << "memcached" << "memcached-usock" << "elasticsearch" << "mongodb";
    return output;
}


QString nginxEntry(WebAppTypes type, QString latestReleaseDir, QString domain, QString serviceName, QString stage, QString port, QString sslPemPath) {
    QString servicePath = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + serviceName;
    QString sslDir = getOrCreateDir(servicePath + DEFAULT_SERVICE_SSLS_DIR);
    QString appProxyContent = readFileContents(latestReleaseDir + DEFAULT_APP_PROXY_FILE).trimmed();
    auto svConfig = new SvdServiceConfig(serviceName);
    if (not sslPemPath.isEmpty()) { /* ssl pem file given */
        logWarn() << "NYI";

    } else {
        logInfo() << "Generating self signed SSL certificate for service:" << serviceName;
        SvdProcess *prc = new SvdProcess("an_self_signed_cert_generate", getuid(), false);

        prc->spawnProcess("test ! -f " + sslDir + domain + ".crt && echo \"EU\nPoland\nWejherowo\nVerKnowSys\nverknowsys.com\n*." + domain + "\nadmin@" + domain + "\n\" | openssl req -new -x509 -nodes -out " + sslDir + domain + ".crt -keyout " + sslDir + domain + ".key");
        prc->waitForFinished(-1);
        prc->deleteLater();
    }
    switch (type) {
        case StaticSite:
            return "\n\
server { \n\
    listen 80; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    location / { \n\
        index index.html index.htm; \n\
        expires 30d; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
}\n\
server { \n\
    listen 443 ssl; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    ssl_certificate " + sslDir + domain + ".crt; \n\
    ssl_certificate_key " + sslDir + domain + ".key; \n\
    location / { \n\
        index index.html index.htm; \n\
        expires 30d; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
}\n";

        case RubySite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server " + DEFAULT_LOCAL_ADDRESS + ":" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header Host $http_host; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
        error_page 400 402 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
} \n\
server { \n\
    listen 443 ssl; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    ssl_certificate " + sslDir + domain + ".crt; \n\
    ssl_certificate_key " + sslDir + domain + ".key; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header X-Forwarded-Proto https; \n\
        proxy_set_header Host $http_host; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
        error_page 400 402 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
} \n";


        case NodeSite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server " + DEFAULT_LOCAL_ADDRESS + ":" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header Host $http_host; \n\
        try_files = $uri /$uri $uri/ @error; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
        error_page 400 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
} \n\
server { \n\
    listen 443 ssl; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "/public; \n\
    ssl_certificate " + sslDir + domain + ".crt; \n\
    ssl_certificate_key " + sslDir + domain + ".key; \n\
    location / { \n\
        proxy_set_header X-Real-IP $remote_addr; \n\
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; \n\
        proxy_set_header X-Forwarded-Proto https; \n\
        proxy_set_header Host $http_host; \n\
        try_files = $uri /$uri $uri/ @error; \n\
        proxy_redirect off; \n\
        if (!-f $request_filename) { \n\
            proxy_pass http://" + serviceName + "-" + stage + "; \n\
        } \n\
        error_page 400 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
} \n";


        case PhpSite: {
            QString prefix = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + serviceName;
writeToFile(prefix + DEFAULT_SERVICE_CONFS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_CONF_FILE, "[global] \n\
    pid = " + prefix + DEFAULT_SERVICE_PIDS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_PID_FILE + " \n\
    error_log = " + prefix + DEFAULT_SERVICE_LOGS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_LOG_FILE + " \n\
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
    listen = " + DEFAULT_LOCAL_ADDRESS + ":" + port + " \n\
    pm = dynamic \n\
    pm.max_children = 8 \n\
    pm.start_servers = 1 \n\
    pm.min_spare_servers = 1 \n\
    pm.max_spare_servers = 2 \n\
    ;pm.process_idle_timeout = 10s; \n\
    ;pm.max_requests = 500 \n");

writeToFile(prefix + DEFAULT_SERVICE_CONFS_DIR + svConfig->releaseName() + "/service.ini", "[PHP] \n\
    docref_root = 0 \n\
    docref_ext = 0 \n\
    engine = On \n\
    short_open_tag = On \n\
    asp_tags = On \n\
    precision = 14 \n\
    y2k_compliance = On \n\
    output_buffering = Off \n\
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
    open_basedir = /tmp:" + latestReleaseDir + ":" + QString(getenv("HOME")) + " \n\
    disable_functions = exec,popen,system \n\
    disable_classes = \n\
    realpath_cache_size = 128k \n\
    expose_php = Off \n\
    max_execution_time = 30 \n\
    max_input_time = 60 \n\
    memory_limit = 512M \n\
    error_reporting = E_ALL & ~E_DEPRECATED \n\
    display_errors = On \n\
    display_startup_errors = On \n\
    log_errors = On \n\
    log_errors_max_len = 1024 \n\
    ignore_repeated_errors = On \n\
    ignore_repeated_source = Off \n\
    report_memleaks = On \n\
    track_errors = On \n\
    html_errors = On \n\
    variables_order = \"GPCS\" \n\
    request_order = \"GP\" \n\
    register_globals = Off \n\
    register_long_arrays = On \n\
    register_argc_argv = On \n\
    auto_globals_jit = On \n\
    post_max_size = 8M \n\
    magic_quotes_gpc = Off \n\
    magic_quotes_runtime = Off \n\
    magic_quotes_sybase = Off \n\
    auto_prepend_file = \n\
    auto_append_file = \n\
    default_mimetype = \"text/html\" \n\
    default_charset = \"UTF-8\" \n\
    doc_root = \n\
    user_dir = \"/tmp:" + latestReleaseDir + "\" \n\
    enable_dl = On \n\
    file_uploads = On \n\
    upload_max_filesize = 50M \n\
    max_file_uploads = 20 \n\
    allow_url_fopen = On \n\
    allow_url_include = Off \n\
    default_socket_timeout = 60 \n\
    cgi.force_redirect = On \n\
[mysql] \n\
    mysql.default_socket=\"" + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/Mysql/" + DEFAULT_SERVICE_SOCKET_FILE + "\" \n\
[Pdo_mysql] \n\
    pdo_mysql.cache_size = 2000 \n\
    pdo_mysql.default_socket= \"" + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/Mysql/" + DEFAULT_SERVICE_SOCKET_FILE + "\" \n\
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
    mysql.allow_persistent = Off \n\
    mysql.cache_size = 2000 \n\
    mysql.max_persistent = -1 \n\
    mysql.max_links = -1 \n\
    mysql.default_port = \n\
    mysql.default_socket = \"" + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/Mysql/" + DEFAULT_SERVICE_SOCKET_FILE + "\" \n\
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
    mysqli.default_socket = \"" + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/Mysql/" + DEFAULT_SERVICE_SOCKET_FILE + "\" \n\
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
    session.cookie_domain = " + domain + " \n\
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
fastcgi_param  SERVER_NAME        $server_name; \n\
fastcgi_param  PHP_ENV            " + stage + "; \n\
fastcgi_param  PHP_APP_NAME       " + serviceName + "; \n\
# fastcgi_param  PHP_ROOT           " + latestReleaseDir + "; \n\
fastcgi_param  PHP_PORT           " + port + "; \n\
fastcgi_param  PHP_DOMAIN         " + domain + "; \n");
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server " + DEFAULT_LOCAL_ADDRESS + ":" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    index index.php; \n\
    try_files $uri $uri/ /index.php; \n\
    location ~ .php$ { \n\
        fastcgi_split_path_info ^(.+\\.php)(.*)$; \n\
        fastcgi_pass " + serviceName + "-" + stage + "; \n\
        fastcgi_index  index.php; \n\
        fastcgi_intercept_errors on; \n\
        fastcgi_param SCRIPT_FILENAME " + latestReleaseDir + "$fastcgi_script_name; \n\
        error_log " + getServiceDataDir(serviceName) + DEFAULT_SERVICE_LOGS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_LOG_FILE + "; \n\
        include " + latestReleaseDir + "/fastcgi_params; \n\
        error_page 400 402 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
        proxy_redirect off; \n\
    } \n\
    location ~ /\\. { \n\
        deny  all; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
}\n\
server { \n\
    listen 443 ssl; \n\
    server_name www." + domain + " " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    ssl_certificate " + sslDir + domain + ".crt; \n\
    ssl_certificate_key " + sslDir + domain + ".key; \n\
    location ~* ^.+.(css|jpg|jpeg|gif|png|js|ico|xml|mp3|eps|cdr|rar|zip|p7|pdf|swf)$ { \n\
        add_header Cache-Control max-age=315360000; \n\
        expires 30d; \n\
        break; \n\
    } \n\
    location / { \n\
        fastcgi_index index.php; \n\
        try_files = $uri /$uri $uri/ /index.php; \n\
        if (-f $request_filename) { \n\
            break; \n\
        } \n\
        include " + latestReleaseDir + "/fastcgi_params; \n\
        fastcgi_param SCRIPT_FILENAME " + latestReleaseDir + "$fastcgi_script_name; \n\
        fastcgi_param PATH_INFO $fastcgi_script_name; \n\
        fastcgi_pass " + serviceName + "-" + stage + "; \n\
        fastcgi_intercept_errors on; \n\
        error_log " + getServiceDataDir(serviceName) + DEFAULT_SERVICE_LOGS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_LOG_FILE + "; \n\
        error_page 400 402 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
        proxy_redirect off; \n\
    } \n\
    location ~ /\\. { \n\
        deny  all; \n\
    } \n\n\
" + appProxyContent + " \n\n\
    access_log off; \n\
}\n";
        }

    }
}


void generateDatastoreSetup(QList<WebDatastore> dbs, QString serviceName, QString stage, WebAppTypes appType) {
    QString databaseName = serviceName + "_" + stage;
    QString servicePath = getServiceDataDir(serviceName);
    QString destinationFile;

    Q_FOREACH(auto db, dbs)
    switch (db) {

        case Postgresql: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + DEFAULT_SHARED_DIR + stage + "/config/database.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  adapter: postgresql \n\
  encoding: unicode \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  pool: 5 \n\
  port: <%= File.read(ENV['HOME'] + \"" + SOFTWARE_DATA_DIR + getDbName(db) + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER + "\") %> \n\
  host: <%= ENV['HOME'] + \"" + SOFTWARE_DATA_DIR + getDbName(db) + "/\" %> \n");
                } break;
                default: break;
            }
        } break;


        case Mysql: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + DEFAULT_SHARED_DIR + stage + "/config/database.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  adapter: mysql2 \n\
  encoding: utf8 \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  socket: <%= File.read(ENV['HOME'] + \"" + SOFTWARE_DATA_DIR + getDbName(db) + DEFAULT_SERVICE_SOCKET_FILE + "\") %> \n\
  host: " + DEFAULT_LOCAL_ADDRESS + " \n");
                } break;
                default: break;
            }
        } break;


        case Mongo: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + DEFAULT_SHARED_DIR + stage + "/config/mongoid.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  sessions: \n\
    default: \n\
      database: " + databaseName + " \n\
      hosts: \n\
        - " + DEFAULT_LOCAL_ADDRESS + ":<%= File.read(ENV['HOME'] + \"" + SOFTWARE_DATA_DIR + getDbName(db) + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER + "\") %> \n");
                } break;
                default: break;
            }
        } break;


        case Redis: {
            /* no conf */
        } break;


        case ElasticSearch: {
            /* no conf */
        } break;


        case Sphinx: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + DEFAULT_SHARED_DIR + stage + "/config/sphinx.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  morphology: stem_en");
                } break;
                default: break;
            }
        } break;


        case NoDB: { /* NoDB fallback to SQLite3 driver */
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + DEFAULT_SHARED_DIR + stage + "/config/database.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  adapter: sqlite3 \n\
  database: db/db_" + databaseName + "_" + stage + ".sqlite3 \n\
  timeout: 5000 \n");
                } break;
                default: break;
            }
        } break;
    }
}


/* consider generating different port for redeployed services soon. */
void generateServicePorts(QString servicePath, int amount) {
    /* generate default port for service */
    if (amount > 100) {
        logWarn() << "Are you serious? You want to reserve more than a hundred ports?";
    }
    if (amount < 1) {
        amount = 1;
    }
    QString portsDir = servicePath + QString(DEFAULT_SERVICE_PORTS_DIR);
    getOrCreateDir(portsDir);
    QString portFilePath = portsDir + QString(DEFAULT_SERVICE_PORT_NUMBER); /* default port */
    // if (not QFile::exists(portFilePath)) {
    QTime midnight(0, 0, 0);
    qsrand(midnight.msecsTo(QTime::currentTime()));
    uint port = registerFreeTcpPort(abs((qrand() + 1024) % 65535));
    logDebug() << "Generated main port:" << QString::number(port);
    writeToFile(portFilePath, QString::number(port));
    // }
    for (int i = 2; i < amount + 1; i++) {
        QString backupPortFilePath = portsDir + QString::number(i - 1);
        qsrand(midnight.msecsTo(QTime::currentTime()));
        int port = registerFreeTcpPort(abs((qrand() + 1024) % 65535));
        writeToFile(backupPortFilePath, QString::number(port));
    }
}


bool validateNginxEntry(QString& servicePath, QString contents) {
    QString prefix = "events { worker_connections 1024; } http { error_log logs/error.log; access_log off; ";
    QString postfix = " }";
    QString uuid = QUuid::createUuid().toString();
    QString uuidFile = servicePath + "/" + uuid;
    QString tmpDir = getOrCreateDir("/tmp/tmp-" + QString::number(getuid()));
    QString testFile = tmpDir + "/proxy.conf-" + uuid;

    QString genContents = contents.replace("listen 80", "listen " + QString::number(registerFreeTcpPort() % 65535));
    genContents = contents.replace("listen 443", "listen " + QString::number(registerFreeTcpPort() % 65535)); /* replace defaul port 80 and 443 with some bogus ports */
    writeToFile(testFile, prefix + genContents + postfix);

    logDebug() << "Generated contents will be validated:" << prefix + genContents + postfix;
    logDebug() << "Validation confirmation UUID:" << uuid << "in file:" << uuidFile;

    getOrCreateDir(tmpDir + "/logs");
    auto clne = new SvdProcess("nginx_entry_validate", getuid(), false);
    clne->spawnProcess("nginx -t -c " + testFile + " -p " + tmpDir + " && touch " + uuidFile);
    clne->waitForFinished(DEFAULT_SERVICE_PAUSE_INTERVAL); /* give it some time */
    clne->deleteLater();

    if (QFile::exists(uuidFile)) {
        logInfo() << "Nginx entry validation passed.";
        logDebug() << "Removing confirmation file:" << uuidFile;
        QFile::remove(uuidFile);
        QFile::remove(testFile);
        return true;
    }
    QFile::remove(testFile);
    return false; /* means failure by definition */
}


void prepareSharedDirs(QString& latestReleaseDir, QString& servicePath, QString& stage) {
    logInfo() << "Preparing shared dir for service start";
    getOrCreateDir(servicePath + DEFAULT_SHARED_DIR + stage + "/public/shared");
    getOrCreateDir(servicePath + DEFAULT_SHARED_DIR + stage + "/log");
    getOrCreateDir(servicePath + DEFAULT_SHARED_DIR + stage + "/tmp");
    getOrCreateDir(servicePath + DEFAULT_SHARED_DIR + stage + "/config");
    getOrCreateDir(latestReleaseDir + "/public");
    logInfo() << "Purging app release /log and /tmp dirs.";
    removeDir(latestReleaseDir + "/log");
    removeDir(latestReleaseDir + "/tmp");
}


void prepareSharedSymlinks(QString& serviceName, QString& latestReleaseDir, QString& stage) {
    auto clne = new SvdProcess("shared_symlinks", getuid(), false);
    logInfo() << "Symlinking and copying shared directory in current release";
    auto svConfig = new SvdServiceConfig(serviceName);
    QString serviceLog = getServiceDataDir(serviceName) + DEFAULT_SERVICE_LOGS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_LOG_FILE;
    logDebug() << "Service log destination:" << serviceLog;
    clne->spawnProcess("cd " + latestReleaseDir + " && test ! -L public/shared && ln -sv ../../../shared/" + stage + "/public/shared public/shared >> " + serviceLog);
    clne->waitForFinished(-1);
    clne->spawnProcess("cd " + latestReleaseDir + " &&\n\
        cd ../../shared/" + stage + "/config/ \n\
        for i in *; do \n\
            cp -v $(pwd)/$i " + latestReleaseDir + "/config/$i >> " + serviceLog + "\n\
        done \n\
    ");
    clne->waitForFinished(-1);
    clne->spawnProcess(" cd " + latestReleaseDir + " && ln -sv ../../shared/" + stage + "/log log >> " + serviceLog);
    clne->waitForFinished(-1);
    clne->spawnProcess("cd " + latestReleaseDir + " && ln -sv ../../shared/" + stage + "/tmp tmp >> " + serviceLog);
    clne->waitForFinished(-1);
    clne->deleteLater();
    svConfig->deleteLater();
}


void cloneRepository(QString& serviceName, QString& branch, QString releaseName) {
    QString repositoryRootPath = QString(getenv("HOME")) + DEFAULT_GIT_REPOSITORY_DIR;
    getOrCreateDir(repositoryRootPath);
    QString sourceRepositoryPath = repositoryRootPath + serviceName + ".git";
    if (not QDir().exists(sourceRepositoryPath)) {
        logError() << "No source git repository found:" << sourceRepositoryPath;
        raise(SIGTERM);
    }
    auto clne = new SvdProcess("clone_repository", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    // if (not QDir(servicePath).exists()) {
    //     logInfo() << "No Web Service dir found:" << servicePath << "Will be created";
    //     getOrCreateDir(servicePath);
    // }

    getOrCreateDir(servicePath + DEFAULT_RELEASES_DIR);

    // logInfo() << "Cleaning old deploys - over count of:" << QString::number(MAX_DEPLOYS_TO_KEEP);
    // QStringList gatheredReleases = QDir(servicePath + DEFAULT_RELEASES_DIR).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    // QStringList releases;
    // if (gatheredReleases.size() > MAX_DEPLOYS_TO_KEEP) {
    //     for (int i = 0; i < MAX_DEPLOYS_TO_KEEP; i++) {
    //         releases << gatheredReleases.at(i);
    //     }
    //     logDebug() << "Releases left:" << releases;
    //     Q_FOREACH(QString release, gatheredReleases) {
    //         if (not releases.contains(release)) {
    //             logDebug() << "Removing old release:" << servicePath + DEFAULT_RELEASES_DIR + release;
    //             clne->spawnProcess("rm -rf " + servicePath + DEFAULT_RELEASES_DIR + release);
    //             clne->waitForFinished(-1);
    //         }
    //     }
    // }

    QString serviceLog = getOrCreateDir(servicePath + DEFAULT_SERVICE_LOGS_DIR + releaseName) + DEFAULT_SERVICE_LOG_FILE;
    QString command = "cd " + servicePath + DEFAULT_RELEASES_DIR +
        "&& sofin reload" +
        "&& git clone " + sourceRepositoryPath + " " + releaseName + " >> " + serviceLog +
        "&& cd " + servicePath + DEFAULT_RELEASES_DIR + releaseName + " " +
        "&& git checkout -b " + branch + " >> " + serviceLog + /* branch might already exists */
        "; git pull origin " + branch + " >> " + serviceLog +
        "&& printf \"Repository update successful in release: " + releaseName + "\n\" >> " + serviceLog;
    logDebug() << "COMMAND:" << command;

    clne->spawnProcess(command);
    clne->waitForFinished(-1);
    logInfo() << "Web app:" << serviceName << "cloned from branch:" << branch;
    clne->deleteLater();
}


void installDependencies(QString& serviceName, QString& latestReleaseDir, QString& releaseName) {
    QString servicePath = getServiceDataDir(serviceName);
    QString conts = "";
    auto dependenciesFile = latestReleaseDir + DEFAULT_SERVICE_DEPENDENCIES_FILE;
    if (QFile::exists(dependenciesFile)) {
        conts = readFileContents(dependenciesFile).trimmed();
    }

    bool installMissing = false;
    Q_FOREACH(auto deps, conts.split("\n")) {
        deps[0] = deps.at(0).toUpper(); /* capitalize */
        QString fileBase = QString(getenv("HOME")) + DEFAULT_USER_APPS_DIR + "/";
        if (not QFile::exists(fileBase + deps + "/" + deps.toLower() + DEFAULT_SERVICE_INSTALLED_EXT)) {
            installMissing = true;

            // XXX: hacks, obviously:
            if (deps.endsWith("magick")) {
                deps = deps.replace("magick", "Magick");
                if (QFile::exists(fileBase + deps + "/" + deps.toLower() + DEFAULT_SERVICE_INSTALLED_EXT)) installMissing = false;
            }
            if (deps.endsWith("-usock")) {
                deps = deps.replace("-usock", "");
                if (QFile::exists(fileBase + deps + "/" + deps.toLower() + DEFAULT_SERVICE_INSTALLED_EXT)) installMissing = false;
            }
        }
        logInfo() << "Checking installation state of dependency:" << deps;
    }

    if (installMissing) {
        logInfo() << "Installing service dependencies:" << conts.replace("\n", ", ");
        auto clne = new SvdProcess("install_dependencies", getuid(), false);
        QString serviceLog = getServiceDataDir(serviceName) + DEFAULT_SERVICE_LOGS_DIR + releaseName + DEFAULT_SERVICE_LOG_FILE;
        clne->spawnProcess("cd " + latestReleaseDir + " && sofin dependencies >> " + serviceLog);
        clne->waitForFinished(-1);
        clne->deleteLater();
    }
}


void requestDependenciesRunningOf(const QString& serviceName, const QStringList appDependencies) {
    auto svConfig = new SvdServiceConfig(serviceName);
    Q_FOREACH(auto val, appDependencies) {
        val[0] = val.at(0).toUpper();
        QString location = getOrCreateDir(getServiceDataDir(val));

        int steps = 0;
        int aPid = readFileContents(location + DEFAULT_SERVICE_PIDS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_PID_FILE).trimmed().toUInt();
        logInfo() << "Requesting dependency presence:" << val << "with pid:" << QString::number(aPid);
        logDebug() << "\\_from:" << location + DEFAULT_SERVICE_PIDS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_PID_FILE;
        while (not pidIsAlive(aPid)) {
            if (not QFile::exists(location + DEFAULT_SERVICE_RUNNING_FILE)) {
                QFile::remove(location + RESTART_TRIGGER_FILE);
                touch(location + RESTART_TRIGGER_FILE);
            }
            sleep(1);
            aPid = readFileContents(location + DEFAULT_SERVICE_PIDS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_PID_FILE).trimmed().toUInt();
            steps++;

            /* check for tcp port of dependency? */
            uint dependencyPort = readFileContents(location + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed().toUInt();
            logDebug() << "Checking depdendency port:" << dependencyPort << "from file:" << location + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER;
            if ((dependencyPort != 0) and
                ((registerFreeTcpPort(dependencyPort) != dependencyPort) or
                 (registerFreeUdpPort(dependencyPort) != dependencyPort))
                ) {
                    logInfo() << "Dependency:" << val << "seems to be running, TCP/UDP port taken:" << QString::number(dependencyPort);
                    break;
            }

            if (steps % 5 == 0) {
                QFile::remove(location + RESTART_TRIGGER_FILE);
                touch(location + RESTART_TRIGGER_FILE);
                logInfo() << "Still waiting for service:" << val << "with pid:" << aPid;
            }
            if (steps > OLD_SERVICE_SHUTDOWN_TIMEOUT) {
                logError() << "Exitting endless loop, cause service:" << val << "refuses to create pid, after " << QString::number(steps -1) << " seconds!";
                break;
            }
        }
        svConfig->deleteLater();
    }
}


QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain) {
    /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
    QString depsFile = latestReleaseDir + DEFAULT_SERVICE_DEPENDENCIES_FILE;
    QString deps = readFileContents(depsFile).trimmed();
    QStringList appDependencies = filterSpawnableDependencies(deps);
    logDebug() << "Gathered dependencies:" << appDependencies << "of size:" << appDependencies.size();
    QString jsonResult = "\"dependencies\": [";

    if (appDependencies.size() == 0) {
        logInfo() << "Empty list of dependencies software, that acts, like some kind of a server.";
        return jsonResult + "], "; /* return empty list */
    }

    for (int indx = 0; indx < appDependencies.size() - 1; indx++) {
        QString elm = appDependencies.at(indx);
        elm[0] = elm.at(0).toUpper();
        jsonResult += "\"" + elm + "\", ";
    }

    QString elmLast = appDependencies.at(appDependencies.size() - 1);
    elmLast[0] = elmLast.at(0).toUpper();
    jsonResult += "\"" + elmLast + "\"], ";
    // jsonResult += QString("\n\n\"configure\": {\"commands\": \"") + "cd SERVICE_PREFIX" + DEFAULT_RELEASES_DIR + "SERVICE_RELEASE" +  + "\"},"; // ; //"svddeployer -n " + serviceName + " -b " + branch + " -o " + domain + "\"},";

    logDebug() << "DEBUG: jsonResult:" << jsonResult;
    return jsonResult;
}


QString buildEnv(QString& serviceName, QStringList deps, QString contentEnv) {
    QString serviceEnvFile = getServiceDataDir(serviceName) + DEFAULT_SERVICE_ENVS_DIR + contentEnv + DEFAULT_SERVICE_ENV_FILE;
    QString result = " ";

    Q_FOREACH(QString fragment, deps) {
        QString serviceDepsFile = getServiceDataDir(fragment) + DEFAULT_SERVICE_ENVS_DIR + contentEnv + DEFAULT_SERVICE_ENV_FILE;
        if (QFile::exists(serviceDepsFile)) {
            QStringList innerContents = readFileContents(serviceDepsFile).trimmed().split('\n');
            logDebug() << "innerCont:" << innerContents;
            Q_FOREACH(QString part, innerContents) {
                result += part + " ";
            }
        }
    }

    if (QFile::exists(serviceEnvFile)) {
        QStringList contents = readFileContents(serviceEnvFile).trimmed().split('\n');
        Q_FOREACH(QString part, contents) {
            result += part + " ";
        }
        logDebug() << "Built env string:" << result;
    }
    return result;
}


QList<WebDatastore> detectDatastores(QString& deps, QString& depsFile) {
    QList<WebDatastore> out;
    QFileInfo bname(depsFile);

    if (deps.trimmed().toLower().contains("postgres")) { /* postgresql specific configuration */
        logInfo() << "Detected Postgresql dependency";
        out << Postgresql;
    }
    if (deps.trimmed().toLower().contains("mysql")) {
        logInfo() << "Detected Mysql dependency";
        out << Mysql;
    }
    if (deps.trimmed().toLower().contains("mongo")) {
        logInfo() << "Detected Mongodb dependency";
        out << Mongo;
    }
    if (deps.trimmed().toLower().contains("sphinx")) {
        logInfo() << "Detected Sphinx dependency";
        out << Sphinx;
    }

    if (out.isEmpty()) {
        logInfo() << "No database driver required.";
        out << NoDB;
    }

    return out;
}


QString getDbName(WebDatastore db) {
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


QStringList filterSpawnableDependencies(const QString& deps) {
    /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
    QStringList allowedToSpawnDeps = getStandaloneDeps(); /* dependencies allowed to spawn as independent service */
    QStringList appDependencies = deps.split("\n");
    logDebug() << "Filtering dependencies:" << appDependencies << "of size:" << appDependencies.size();

    /* filter forbiddens */
    for (int i = 0; i < appDependencies.size(); i++) {
        QString d1 = appDependencies.at(i);
        if (not allowedToSpawnDeps.contains(d1))
            appDependencies[i] = "";
    }
    appDependencies.removeAll("");

    logDebug() << "Filtered dependencies:" << appDependencies << "of size:" << appDependencies.size();
    return appDependencies;
}


void startWithoutDependencies(const QString& servicePath) {
    logInfo() << "Launching service without dependencies, using newly generated igniter.";
    if (not QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
        touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);
    else {
        QFile::remove(servicePath + DEFAULT_SERVICE_RUNNING_FILE);
        touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);
    }
}


void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch) {

    QString servicePath = getServiceDataDir(serviceName);

    logDebug() << "Creating web-app environment";
    QString domainFilePath = servicePath + DEFAULT_SERVICE_DOMAINS_DIR + domain;
    logInfo() << "Writing domain:" << domain << "to file:" << domainFilePath;
    touch(servicePath + DEFAULT_SERVICE_DOMAINS_DIR + domain);

    /* create "deploying" state */
    touch(servicePath + DEFAULT_SERVICE_DEPLOYING_FILE);
    logDebug() << "Created deploying state in file:" << servicePath + DEFAULT_SERVICE_DEPLOYING_FILE << "for service:" << serviceName;

    auto latestRelease = "build-in-progress-" + stage;
    auto latestReleaseDir = servicePath + DEFAULT_RELEASES_DIR + latestRelease;
    cloneRepository(serviceName, branch, latestRelease);
    logDebug() << "Writing domain file:" << domain << " of service with path:" << servicePath;
    touch(servicePath + DEFAULT_SERVICE_DOMAINS_DIR + domain); // XXX
    installDependencies(serviceName, latestReleaseDir, latestRelease);
    logDebug() << "Release build in progress files:\n" << QDir(latestReleaseDir).entryList();
    logDebug() << "Release path:" << latestReleaseDir;
    auto appDetector = new WebAppTypeDetector(latestReleaseDir);
    auto appType = appDetector->getType();
    QString envEntriesString = "";
    logDebug() << "Detected application type:" << appDetector->typeName;
    delete appDetector;

    /* do app type specific action */
    auto clne = new SvdProcess("create_environment", getuid(), false);
    QStringList appDependencies;
    QString jsonResult = "";


    QString databaseName = serviceName + "_" + stage;
    QString depsFile = latestReleaseDir + DEFAULT_SERVICE_DEPENDENCIES_FILE;
    QString deps = "";
    if (QFile::exists(depsFile)) { /* NOTE: special software list file from Sofin, called ".dependencies" */
        deps = readFileContents(depsFile).trimmed();
    }
    QList<WebDatastore> datastores;
    datastores = detectDatastores(deps, depsFile);

    QString envFilePath = getOrCreateDir(servicePath + DEFAULT_SERVICE_ENVS_DIR + latestRelease) + DEFAULT_SERVICE_ENV_FILE;

    switch (appType) {

        case StaticSite: {

            jsonResult = "{\"alwaysOn\": false, \"watchPort\": false, \"webApp\": true, ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            jsonResult += "\n\n\"start\": {\"commands\": \"echo 'Static app ready' >> SERVICE_LOG\"}\n}";

            generateServicePorts(servicePath);
            QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();

            /* write to service env file */

            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "STATIC_ENV=" + stage + "\n";
            envEntriesString += "STATIC_APP_NAME=" + serviceName + "\n";
            // envEntriesString += "STATIC_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "STATIC_PORT=" + servPort + "\n";
            envEntriesString += "STATIC_DOMAIN=" + domain + "\n";
            writeToFile(envFilePath, envEntriesString);

        } break;


        case RubySite: {

            generateServicePorts(servicePath);
            QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();

            /* write to service env file */
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SERVICE_SSLS_DIR + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";
            envEntriesString += "RUBY_ENV=" + stage + "\n";
            envEntriesString += "RUBY_APP_NAME=" + serviceName + "\n";
            // envEntriesString += "RUBY_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "RUBY_PORT=" + servPort + "\n";
            envEntriesString += "RUBY_DOMAIN=" + domain + "\n";
            writeToFile(envFilePath, envEntriesString);

            appDependencies = filterSpawnableDependencies(deps);

            jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"softwareName\": \"Ruby\", \"webApp\": true, ";
            QString environment = buildEnv(serviceName, appDependencies, latestRelease);
            // logDebug() << "Generateed Service Environment:" << environment;
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            // QString startResultJson = ""; /* command that actually launches main app */
            QString serviceLog = servicePath + DEFAULT_SERVICE_LOGS_DIR + latestRelease + DEFAULT_SERVICE_LOG_FILE;

            QMap<QString, QString> serviceWorkers; /* additional workers of service: (startCommands, stopCommands) */
            QString procFile = latestReleaseDir + "/Procfile"; /* heroku compatible procfile */
            if (QFile::exists(procFile)) {
                QStringList entries = readFileContents(procFile).trimmed().split("\n");
                logInfo() << "Proceeding with Procfile entries:" << entries;

                QString svPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
                Q_FOREACH(QString entry, entries) {
                    QString procfileHead = entry.split(":").at(0);
                    QString procfileTail = entry.split(":").at(1);
                    procfileTail = procfileTail.replace("$PORT", "SERVICE_PORT"); /* replace $PORT value of Procfile if exists */

                    if (procfileHead == "web") { /* web worker is defined here */
                        logInfo() << "Found web worker:" << procfileHead;
                        logDebug() << "Worker entry:" << procfileTail << "on port:" << svPort;
                        // startResultJson += " ";
                        /* NOTE: dropped -b " + DEFAULT_LOCAL_ADDRESS + ", cause rack isn't supporting this feature like rails app, it should be explicitly given in Procfile then. */

                        /*
                           XXX: now we need to try to figure out how to launch that app
                           (which is just fucking stupid, but there's no standard way to launch anything here),
                           unfortunately Rake and/or Rails teams can't talk with each other to solve different
                           arguments accepted by both services:
                           --daemon         vs --daemonize
                           -D               vs -d
                           --environment    vs --env
                           --host           vs --binding
                           -b               vs -o
                           So we need to hack it!
                           Whole <3 goes to Rack/Rails teams!
                         */
                        /* Rack is deafault pick */
                        QString daemOpt = "-D";
                        QString envOpt = "-E";
                        QString bindOpt = "-o";
                        if (procfileTail.toLower().contains("rails")) {
                            logInfo() << "Rails launcher detected.";
                            daemOpt = "-d";
                            envOpt = "-e";
                            bindOpt = "-b";
                        } else
                            logInfo() << "Rack launcher detected.";

                        serviceWorkers.insert(
                            /* start commands: */
                            QString("sofin reload && cd SERVICE_PREFIX") + DEFAULT_RELEASES_DIR + "SERVICE_RELEASE && \n" + buildEnv(serviceName, appDependencies, latestRelease) + " " + procfileTail + " -p SERVICE_PORT -P SERVICE_PID " + envOpt + " " + stage + " " + bindOpt + " " + DEFAULT_LOCAL_ADDRESS + " " + daemOpt + " >> SERVICE_LOG 2>&1 && echo " + domain + " > /Public/" + serviceName + "_" + getenv("USER") + ".web-app",

                            /* stop commands */
                            "" //svddw $(cat SERVICE_PID) >> SERVICE_LOG"
                        );

                    } else {
                        logInfo() << "Found an entry:" << procfileHead;
                        // QString procPidFile = "/" + procfileHead + ".pid";

                        serviceWorkers.insert( /* NOTE: by default, each worker must accept pid location, log location and daemon mode */

                            /* (start commands, stop commands) : */
                            QString("sofin reload && cd SERVICE_PREFIX") + DEFAULT_RELEASES_DIR + "SERVICE_RELEASE && \n" + buildEnv(serviceName, appDependencies, latestRelease) + " bundle exec " + procfileTail + " -P SERVICE_PID -L SERVICE_LOG" + "-" + procfileHead + " -d && \n echo 'Started worker " + procfileHead + "' >> SERVICE_LOG",

                            /* , stop commands) : */
                            "" //svddw $(cat SERVICE_PID) >> SERVICE_LOG"

                        );
                    }
                }

                /* generate correct order of application execution after workers */
                jsonResult += QString("\n\n\"start\": {\"commands\": \"");
                Q_FOREACH(QString part, serviceWorkers.keys()) { /* keys => start commands */
                    jsonResult += part + " &&\n";
                }
                jsonResult += " true ;";
                jsonResult += "\"}, \n\n\"stop\": {\"commands\": \"";
                Q_FOREACH(QString acmd, serviceWorkers.keys()) {
                    logDebug() << "ACMD:" << acmd;
                    QString cmd = serviceWorkers.take(acmd);
                    jsonResult += cmd + " ;\n";
                }
                jsonResult += "\"}\n}";

            } else { /* generate standard igniter entry */

                logInfo() << "Generating default entry (no Procfile used)";
                jsonResult += QString("\n\n\"start\": {\"commands\": \"sofin reload && cd SERVICE_PREFIX") + DEFAULT_RELEASES_DIR + "SERVICE_RELEASE && \n" + buildEnv(serviceName, appDependencies, latestRelease) + " bundle exec rails s -b " + DEFAULT_LOCAL_ADDRESS + " -p SERVICE_PORT -P SERVICE_PID >> SERVICE_LOG &\"}\n}";
            }
            logDebug() << "Generated Igniter JSON:" << jsonResult;

            QString cacertLocation = QString(DEFAULT_CA_CERT_ROOT_SITE) + DEFAULT_SSL_CA_FILE;
            logInfo() << "Gathering SSL CA certs from:" << cacertLocation << "if necessary.";
            clne->spawnProcess("cd " + servicePath + " && test ! -f " + DEFAULT_SSL_CA_FILE + " && curl -C - -L -O " + cacertLocation + " >> " + serviceLog);
            clne->waitForFinished(-1);

        } break;


        case NodeSite: {

            generateServicePorts(servicePath, 2); /* XXX: 2 ports for node by default */
            QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();

            /* write to service env file */
            QString websocketsPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + "/1").trimmed(); // XXX: hardcoded
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "NODE_APP_NAME=" + serviceName + "\n";
            // envEntriesString += "NODE_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "NODE_ENV=" + stage + "\n";
            envEntriesString += "NODE_PORT=" + servPort + "\n";
            envEntriesString += "NODE_DOMAIN=" + domain + "\n";
            envEntriesString += "NODE_WEBSOCKET_PORT=" + websocketsPort + "\n";
            envEntriesString += "NODE_WEBSOCKET_CHANNEL_NAME=" + serviceName + "-" + domain + "\n";
            writeToFile(envFilePath, envEntriesString);

            QString depsFile = latestReleaseDir + DEFAULT_SERVICE_DEPENDENCIES_FILE;
            QString deps = readFileContents(depsFile).trimmed();
            appDependencies = filterSpawnableDependencies(deps);

            jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"softwareName\": \"Node\", \"webApp\": true, \"portsPool\": 2, ";
            QString environment = buildEnv(serviceName, appDependencies, latestRelease);
            logDebug() << "Generateed Service Environment:" << environment;
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            jsonResult += QString("\n\n\"start\": {\"commands\": \"sofin reload && cd SERVICE_PREFIX") + DEFAULT_RELEASES_DIR + "SERVICE_RELEASE && \n" + buildEnv(serviceName, appDependencies, latestRelease) + "bin/app >> SERVICE_LOG 2>&1 &\"}\n}"; /* bin/app has to get all settings from ENV (stage in NODE_ENV) */
            logDebug() << "Generated Igniter JSON:" << jsonResult;

            logInfo() << "Installing npm modules for stage:" << stage << "of Node Site";
            clne->spawnProcess(QString("cd SERVICE_PREFIX") + DEFAULT_RELEASES_DIR + "SERVICE_RELEASE && \n" + buildEnv(serviceName, appDependencies, latestRelease) + " npm install >> SERVICE_LOG");
            clne->waitForFinished(-1);

        } break;


        case PhpSite: {

            jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"softwareName\": \"Php\", \"webApp\": true, ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            #ifdef __APPLE__
                logError() << "Apple PHP deployments aren't supported yet!";
                raise(SIGTERM);
            #endif

            generateServicePorts(servicePath);
            QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();

            QString depsFile = latestReleaseDir + DEFAULT_SERVICE_DEPENDENCIES_FILE;
            QString deps = readFileContents(depsFile).trimmed();
            appDependencies = filterSpawnableDependencies(deps);

            jsonResult += "\n\n\"start\": {\"commands\": \"sofin reload && " + buildEnv(serviceName, appDependencies, latestRelease) + " SERVICE_ROOT/exports/php-fpm -c SERVICE_PREFIX/service.ini --fpm-config SERVICE_CONF -D && \n echo 'Php app ready' >> SERVICE_LOG\"}\n}";

        } break;


    }

    /* write igniter to user igniters */
    QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
    logDebug() << "Generating igniter:" << igniterFile;
    writeToFile(igniterFile, jsonResult);

    removeDir(latestReleaseDir);
    /* ---- the magic barrier after igniter is defined. we'll have stable config->releaseName() ---- */

    auto svConfig = new SvdServiceConfig(serviceName);
    cloneRepository(serviceName, branch, svConfig->releaseName());

    // // TODO: keep also history file here
    // logInfo() << "Writing web-app previous release pid";
    // auto conts = readFileContents(servicePath + DEFAULT_SERVICE_PIDS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_PID_FILE).trimmed();
    // if (conts.length() > 0)
    //     writeToFile(servicePath + DEFAULT_SERVICE_PREVIOUS_RELEASE_FILE, conts);
    // startWithoutDependencies(servicePath);

    logDebug() << "Creating .pids, .envs and .confs";
    getOrCreateDir(getServiceDataDir(serviceName) + DEFAULT_SERVICE_PIDS_DIR + svConfig->releaseName());
    getOrCreateDir(getServiceDataDir(serviceName) + DEFAULT_SERVICE_ENVS_DIR + svConfig->releaseName());
    getOrCreateDir(getServiceDataDir(serviceName) + DEFAULT_SERVICE_CONFS_DIR + svConfig->releaseName());
    getOrCreateDir(getServiceDataDir(serviceName) + DEFAULT_SERVICE_LOGS_DIR + svConfig->releaseName());

    logInfo() << "Moving rebuilt prefix from:" << latestReleaseDir;
    // auto moveProcess = new SvdProcess("move_built_web_app", getuid(), false);
    QString oldRD = latestReleaseDir;
    removeDir(oldRD);
    latestReleaseDir = servicePath + DEFAULT_RELEASES_DIR + svConfig->releaseName();


    // TODO: launch garbage collector:
    // 1. remove all empty .pids, .envs, .confs dirs
    // 2. remove all non empty .pids .envs .confs from old releases

    QString servPort = readFileContents(getServiceDataDir(serviceName) + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();

    /* now we can generate environment again for destination app */
    envEntriesString = "";
    QString envFilePathDest = getOrCreateDir(servicePath + DEFAULT_SERVICE_ENVS_DIR + svConfig->releaseName()) + DEFAULT_SERVICE_ENV_FILE;
    switch (appType) {
        case StaticSite: {
            /* write to service env file */
            logInfo() << "Building environment for stage:" << stage << "in file:" << envFilePathDest;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "STATIC_ENV=" + stage + "\n";
            envEntriesString += "STATIC_APP_NAME=" + serviceName + "\n";
            envEntriesString += "STATIC_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "STATIC_PORT=" + servPort + "\n";
            envEntriesString += "STATIC_DOMAIN=" + domain + "\n";
            writeToFile(envFilePathDest, envEntriesString);

        } break;
        case RubySite: {
            /* write to service env file */
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SERVICE_SSLS_DIR + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";
            envEntriesString += "RUBY_ENV=" + stage + "\n";
            envEntriesString += "RUBY_APP_NAME=" + serviceName + "\n";
            envEntriesString += "RUBY_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "RUBY_PORT=" + servPort + "\n";
            envEntriesString += "RUBY_DOMAIN=" + domain + "\n";
            writeToFile(envFilePathDest, envEntriesString);

        } break;
        case NodeSite: {
            QString websocketsPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + "/1").trimmed(); // XXX: hardcoded
            /* write to service env file */
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "NODE_APP_NAME=" + serviceName + "\n";
            envEntriesString += "NODE_ENV=" + stage + "\n";
            envEntriesString += "NODE_PORT=" + servPort + "\n";
            envEntriesString += "NODE_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "NODE_DOMAIN=" + domain + "\n";
            envEntriesString += "NODE_WEBSOCKET_PORT=" + websocketsPort + "\n";
            envEntriesString += "NODE_WEBSOCKET_CHANNEL_NAME=" + serviceName + "-" + domain + "\n";
            writeToFile(envFilePathDest, envEntriesString);

        } break;
        case PhpSite: {

        } break;
    }

    QString serviceLog = getServiceDataDir(serviceName) + DEFAULT_SERVICE_LOGS_DIR + svConfig->releaseName() + DEFAULT_SERVICE_LOG_FILE;
    // moveProcess->spawnProcess("cp -R " + oldRD + "/ " + latestReleaseDir);
    // moveProcess->waitForFinished(-1);
    logInfo() << "to:" << latestReleaseDir;
    // moveProcess->deleteLater();
    // removeDir(oldRD);

    /* TODO: clean generated environment */

    requestDependenciesRunningOf(serviceName, appDependencies);
    prepareSharedDirs(latestReleaseDir, servicePath, stage);
    generateDatastoreSetup(datastores, serviceName, stage, appType);
    prepareSharedSymlinks(serviceName, latestReleaseDir, stage);

    Q_FOREACH(auto datastore, datastores) {
        logInfo() << "Running datastore setup for engine:" << getDbName(datastore);
        switch (datastore) {
            case Postgresql: {
                logDebug() << "Creating user:" << databaseName;
                clne->spawnProcess("createuser -s -d -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + getDbName(datastore) + " -p $(sofin port " + getDbName(datastore) + ") " + databaseName + " >> " + serviceLog);
                clne->waitForFinished(-1);
                logDebug() << "Creating datastore:" << databaseName;
                clne->spawnProcess("createdb -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + getDbName(datastore) + " -p $(sofin port " + getDbName(datastore) + ") -O " + databaseName + " " + databaseName + " >> " + serviceLog);
                clne->waitForFinished(-1);

            } break;

            default: break;

        }
    }

    logDebug() << "Setting configured state for service:" << serviceName;
    touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);
    logInfo() << "Finalizing environment setup of service:" << serviceName;

    switch (appType) {
        case RubySite:
            if (QFile::exists(latestReleaseDir + "/Gemfile")) {
                logInfo() << "Installing bundle for stage:" << stage << "of Ruby Site";
                getOrCreateDir(servicePath + "/bundle-" + stage);
                clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies, svConfig->releaseName()) + " bundle install --path " + servicePath + "/bundle-" + stage + " --without test development >> " + serviceLog);
                clne->waitForFinished(-1);
            }
            if (QFile::exists(latestReleaseDir + "/Rakefile")) {
                logInfo() << "Rakefile found, running database migrations";
                if (not datastores.contains(Postgresql)) { /* postgresql db creation is already done before this hook */
                    clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies, svConfig->releaseName()) + " bundle exec rake db:create >> " + serviceLog);
                    clne->waitForFinished(-1);
                }
                clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies, svConfig->releaseName()) + " bundle exec rake db:migrate >> " + serviceLog);
                clne->waitForFinished(-1);
            } else
                logInfo() << "No Rakefile found. Skipping standard rake tasks";

            if (QFile::exists(latestReleaseDir + "/Rakefile") and QDir().exists(latestReleaseDir + "/app/assets")) {
                logInfo() << "Building assets for web-app:" << serviceName;
                clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies, svConfig->releaseName()) + " bundle exec rake assets:precompile >> " + serviceLog);
                clne->waitForFinished(-1);
            }
            break;

        case NodeSite:
            break;

        default:
            break;

    }

        /* spawn bin/build */
    logInfo() << "Invoking bin/build of project if exists";
    clne->spawnProcess("cd " + latestReleaseDir + " && test -x bin/build && " + buildEnv(serviceName, appDependencies, svConfig->releaseName()) + " bin/build " + stage + " >> " + serviceLog);
    clne->waitForFinished(-1);
    /* -- */

    /* prepare http proxy */
    logInfo() << "Generating http proxy configuration for web-app";
    QString port = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
    QString contents = nginxEntry(appType, latestReleaseDir, domain, serviceName, stage, port);
    if (validateNginxEntry(servicePath, contents)) {
        logDebug() << "Generated proxy contents:" << contents;
        writeToFile(servicePath + DEFAULT_PROXY_FILE, contents);
    } else {
        logWarn() << "Web-App proxy autogeneration failed. It might be a failure in generated nginx proxy file or user input. Proxy file generation skipped!";
    }
    /* -- */

    if (not QFile::exists(servicePath + AUTOSTART_TRIGGER_FILE))
        touch(servicePath + AUTOSTART_TRIGGER_FILE);

    // logInfo() << "Writing web-app current release version";
    // writeToFile(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE, svConfig->releaseName());
    // startWithoutDependencies(serviceName);
    clne->deleteLater();
    svConfig->deleteLater();
}
