/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "deploy.h"


QString nginxEntry(WebAppTypes type, QString latestReleaseDir, QString domain, QString serviceName, QString stage, QString port) {
    switch (type) {
        case StaticSite:
            return "\n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
    location / { \n\
        index index.html index.htm; \n\
        expires 30d; \n\
    } \n\
    access_log off; \n\
}\n";

        case RubySite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server " + DEFAULT_LOCAL_ADDRESS + ":" + port + "; \n\
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
        error_page 400 402 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
    } \n\
    access_log off; \n\
} \n";


        case NodeSite:
            return "\n\
upstream " + serviceName + "-" + stage + " { \n\
    server " + DEFAULT_LOCAL_ADDRESS + ":" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
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
    } \n\
    access_log off; \n\
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
    listen = " + DEFAULT_LOCAL_ADDRESS + ":" + port + " \n\
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
    server " + DEFAULT_LOCAL_ADDRESS + ":" + port + "; \n\
} \n\
server { \n\
    listen 80; \n\
    server_name " + domain + "; \n\
    root " + latestReleaseDir + "; \n\
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
        error_page 400 402 403 404 502 503 504 = error.html; \n\
    } \n\
    location /error.html { \n\
        index error.html; \n\
        proxy_redirect off; \n\
    } \n\
    location ~ /\\. { \n\
        deny  all; \n\
    } \n\
    access_log off; \n\
}\n";
        }

        case NoType:
            return "";
    }
}


void generateDatastoreSetup(WebDatastore db, QString serviceName, QString stage, WebAppTypes appType) {
    QString databaseName = serviceName + "-" + stage;
    QString servicePath = getServiceDataDir(serviceName);
    QString destinationFile;

    switch (db) {

        case Postgresql: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + "/shared/" + stage + "/config/database.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  adapter: postgresql \n\
  encoding: unicode \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  pool: 5 \n\
  port: <%= File.read(ENV['HOME'] + \"/SoftwareData/" + getDbName(db) + "/.ports/0\") %> \n\
  host: <%= ENV['HOME'] + \"/SoftwareData/" + getDbName(db) + "/\" %> \n");
                } break;
                default: break;
            }
        } break;


        case Mysql: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + "/shared/" + stage + "/config/database.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  adapter: mysql2 \n\
  encoding: utf8 \n\
  database: " + databaseName + " \n\
  username: " + databaseName + " \n\
  socket: <%= File.read(ENV['HOME'] + \"/SoftwareData/" + getDbName(db) + "/service.sock\") %> \n\
  host: " + DEFAULT_LOCAL_ADDRESS + " \n");
                } break;
                default: break;
            }
        } break;


        case Mongo: {
            switch (appType) {
                case RubySite: {
                    destinationFile = servicePath + "/shared/" + stage + "/config/mongoid.yml";
                    if (not QFile::exists(destinationFile))
                        writeToFile(destinationFile,
stage + ": \n\
  sessions: \n\
    default: \n\
      database: " + databaseName + " \n\
      hosts: \n\
        - " + DEFAULT_LOCAL_ADDRESS + ":<%= File.read(ENV['HOME'] + \"/SoftwareData/" + getDbName(db) + "/.ports/0\") %> \n");
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
                    destinationFile = servicePath + "/shared/" + stage + "/config/sphinx.yml";
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
                    destinationFile = servicePath + "/shared/" + stage + "/config/database.yml";
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
    if (not QFile::exists(portFilePath)) {
        int port = registerFreeTcpPort(abs((rand() + 1024) % 65535));
        logDebug() << "Generated main port:" << QString::number(port);
        writeToFile(portFilePath, QString::number(port));
    }
    for (int i = 2; i < amount + 1; i++) {
        QString backupPortFilePath = portsDir + QString::number(i - 1);
        if (not QFile::exists(backupPortFilePath)) {
            int port = registerFreeTcpPort(abs((rand() + 1024) % 65535));
            logDebug() << "Generated additional port:" << QString::number(port);
            writeToFile(backupPortFilePath, QString::number(port));
        }
    }
}


bool validateNginxEntry(QString& servicePath, QString contents) {
    QString prefix = "events { worker_connections 1024; } http { error_log off; access_log off; ";
    QString postfix = " }";
    QString uuid = QUuid::createUuid().toString();
    QString uuidFile = servicePath + "/" + uuid;
    QString testFile = "/tmp/proxy.conf-" + uuid;

    QString genContents = contents.replace("listen 80", "listen " + QString::number(registerFreeTcpPort())); /* replace defaul port 80 with some bogus port */
    writeToFile(testFile, prefix + genContents + postfix);

    logDebug() << "Generated contents will be validated:" << prefix + genContents + postfix;
    logDebug() << "Validation confirmation UUID:" << uuid << "in file:" << uuidFile;

    getOrCreateDir("/tmp/logs");
    auto clne = new SvdProcess("nginx_entry_validate", getuid(), false);
    clne->spawnProcess("nginx -t -c " + testFile + " -p /tmp && touch " + uuidFile);
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


void prepareHttpProxy(QString& servicePath, WebAppTypes appType, QString& latestReleaseDir, QString& domain, QString& serviceName, QString& stage) {
    logInfo() << "Generating http proxy configuration";
    QString port = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
    QString contents = nginxEntry(appType, latestReleaseDir, domain, serviceName, stage, port);
    if (validateNginxEntry(servicePath, contents)) {
        logDebug() << "Generated proxy contents:" << contents;
        writeToFile(servicePath + DEFAULT_PROXY_FILE, contents);
    } else {
        logWarn() << "Web-App Validation failed, cause of a failure in generated nginx proxy file. Proxy file generation skipped!";
    }
}


void prepareSharedDirs(QString& latestReleaseDir, QString& servicePath, QString& stage) {
    logInfo() << "Preparing shared dir for service start";
    getOrCreateDir(servicePath + "/shared/" + stage + "/public/shared"); /* /public usually exists */
    getOrCreateDir(servicePath + "/shared/" + stage + "/log");
    getOrCreateDir(servicePath + "/shared/" + stage + "/tmp");
    getOrCreateDir(servicePath + "/shared/" + stage + "/config");
    getOrCreateDir(latestReleaseDir + "/public");
    logInfo() << "Purging app release /log and /tmp dirs.";
    removeDir(latestReleaseDir + "/log");
    removeDir(latestReleaseDir + "/tmp");
}


void prepareSharedSymlinks(QString& latestReleaseDir, QString& servicePath, QString& stage) {
    auto clne = new SvdProcess("shared_symlinks", getuid(), false);
    logInfo() << "Symlinking and copying shared directory in current release";
    clne->spawnProcess("cd " + latestReleaseDir + " && ln -sv ../../../shared/" + stage + "/public/shared public/shared >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->spawnProcess("cd " + latestReleaseDir + " &&\n\
        cd ../../shared/" + stage + "/config/ \n\
        for i in *; do \n\
            cp -v $(pwd)/$i " + latestReleaseDir + "/config/$i >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 \n\
        done \n\
    ");
    clne->waitForFinished(-1);
    clne->spawnProcess(" cd " + latestReleaseDir + " && ln -sv ../../shared/" + stage + "/log log >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->spawnProcess("cd " + latestReleaseDir + " && ln -sv ../../shared/" + stage + "/tmp tmp >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


void cloneRepository(QString& sourceRepositoryPath, QString& serviceName, QString& branch) {
    if (not QDir().exists(sourceRepositoryPath)) {
        logError() << "No source git repository found:" << sourceRepositoryPath;
        raise(SIGTERM);
    }
    auto clne = new SvdProcess("clone_repository", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    if (not QDir().exists(servicePath)) {
        logInfo() << "No Web Service dir found:" << servicePath << "Will be created";
        getOrCreateDir(servicePath);
    }

    /* create "deploying" state */
    touch(servicePath + DEFAULT_SERVICE_DEPLOYING_FILE);
    logDebug() << "Created deploying state in file:" << servicePath + DEFAULT_SERVICE_DEPLOYING_FILE << "for service:" << serviceName;

    getOrCreateDir(servicePath + "/releases/");

    logInfo() << "Cleaning old deploys - over count of:" << QString::number(MAX_DEPLOYS_TO_KEEP);
    QStringList gatheredReleases = QDir(servicePath + "/releases/").entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    QStringList releases;
    if (gatheredReleases.size() > MAX_DEPLOYS_TO_KEEP) {
        for (int i = 0; i < MAX_DEPLOYS_TO_KEEP; i++) {
            releases << gatheredReleases.at(i);
        }
        logDebug() << "Releases left:" << releases;
        Q_FOREACH(QString release, gatheredReleases) {
            if (not releases.contains(release)) {
                logDebug() << "Removing old release:" << servicePath + "/releases/" + release;
                clne->spawnProcess("rm -rf " + servicePath + "/releases/" + release);
                clne->waitForFinished(-1);
            }
        }
    }

    QString command = QString("export DATE=\"app-$(date +%d%m%Y-%H%M%S)\"") +
        "&& cd " + servicePath + " > " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& sofin reload > " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& git clone " + sourceRepositoryPath + " releases/${DATE}" + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "&& cd " + servicePath + "/releases/${DATE} " + " 2>&1 " +
        "&& git checkout -b " + branch + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " + /* branch might already exists */
        "; git pull origin " + branch + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 " +
        "; cat " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE + " > " + servicePath + DEFAULT_SERVICE_PREVIOUS_RELEASE_FILE +
        "; cat " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE + " >> " + servicePath + DEFAULT_SERVICE_RELEASES_HISTORY +
        "; printf \"${DATE}\" > " + servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE +
        "&& printf \"Repository update successful in release ${DATE}\" >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ";
    logDebug() << "COMMAND:" << command;

    clne->spawnProcess(command);
    clne->waitForFinished(-1);
    logInfo() << "Web app:" << serviceName << "cloned on branch:" << branch;
    clne->deleteLater();
}


void installDependencies(QString& serviceName) {
    /* setting up service domain */
    auto clne = new SvdProcess("install_dependencies", getuid(), false);
    QString servicePath = getServiceDataDir(serviceName);
    logInfo() << "Installing service dependencies";
    auto latestRelease = readFileContents(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE).trimmed();
    auto latestReleaseDir = servicePath + "/releases/" + latestRelease;

    clne->spawnProcess("cd " + latestReleaseDir + " && sofin dependencies >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


void spawnBinBuild(QString& latestReleaseDir, QString& serviceName, QString& servicePath, QStringList appDependencies) {
    auto clne = new SvdProcess("spawn_bin_build", getuid(), false);
    logInfo() << "Invoking bin/build of project (if exists)";
    clne->spawnProcess("cd " + latestReleaseDir + " && test -x bin/build && " + buildEnv(serviceName, appDependencies) + " bin/build >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
    clne->waitForFinished(-1);
    clne->deleteLater();
}


QString generateIgniterDepsBase(QString& latestReleaseDir, QString& serviceName, QString& branch, QString& domain) {
    QStringList allowedToSpawnDeps; /* dependencies allowed to spawn as independenc service */
    allowedToSpawnDeps << "postgresql" << "mysql" << "redis" << "redis-usock" << "nginx" << "passenger" << "sphinx" << "memcached" << "elasticsearch"; // XXX: hardcoded
    // TODO: define Sphinx igniter

    QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
    QString deps = readFileContents(depsFile).trimmed();

    /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
    QStringList appDependencies = deps.split("\n");
    logDebug() << "Gathered dependencies:" << appDependencies << "of size:" << appDependencies.size();
    QString jsonResult = "\"dependencies\": [";

    /* filter forbiddens */
    for (int i = 0; i < appDependencies.size(); i++) {
        QString d1 = appDependencies.at(i);
        if (not allowedToSpawnDeps.contains(d1))
            appDependencies[i] = "";
    }
    appDependencies.removeAll("");

    if (appDependencies.size() == 0) {
        logInfo() << "Empty list of dependencies software, that acts, like some kind of a server.";
        return jsonResult + "], "; /* return empty list */
    }


    Q_FOREACH(auto val, appDependencies) {
        val[0] = val.at(0).toUpper();
        QString location = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + val;
        getOrCreateDir(location);
        QFile::remove(location + START_TRIGGER_FILE);
        touch(location + START_TRIGGER_FILE);

        int steps = 0;
        while (not QFile::exists(location + DEFAULT_SERVICE_RUNNING_FILE)) {
            logDebug() << "Still waiting for service:" << val;
            sleep(1);
            steps++;
            if (steps > OLD_SERVICE_SHUTDOWN_TIMEOUT) {
                logError() << "Exitting endless loop, cause service:" << val << "refuses to go down after " << QString::number(steps -1) << " seconds!";
                break;
            }
        }
    }

    for (int indx = 0; indx < appDependencies.size() - 1; indx++) {
        QString elm = appDependencies.at(indx);
        elm[0] = elm.at(0).toUpper();
        logInfo() << "Spawning depdendency:" << elm;
        QString location = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + elm;
        getOrCreateDir(location);
        QFile::remove(location + START_TRIGGER_FILE);
        touch(location + START_TRIGGER_FILE);
        jsonResult += "\"" + elm + "\", ";
    }

    QString elmLast = appDependencies.at(appDependencies.size() - 1);
    elmLast[0] = elmLast.at(0).toUpper();
    jsonResult += "\"" + elmLast + "\"], ";
    jsonResult += QString("\n\n\"configure\": {\"commands\": \"") + "svddeployer -n " + serviceName + " -b " + branch + " -o " + domain + "\"},";

    logInfo() << "Spawning depdendency:" << elmLast;
    QString location = QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + elmLast;
    getOrCreateDir(location);
    QFile::remove(location + START_TRIGGER_FILE);
    touch(location + START_TRIGGER_FILE);
    logDebug() << "DEBUG: jsonResult:" << jsonResult;
    return jsonResult;
}


QString buildEnv(QString& serviceName, QStringList deps) {
    QString serviceEnvFile = getServiceDataDir(serviceName) + DEFAULT_SERVICE_ENV_FILE;
    QString result = " ";

    Q_FOREACH(QString fragment, deps) {
        QString serviceDepsFile = getServiceDataDir(fragment) + DEFAULT_SERVICE_ENV_FILE;
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


WebDatastore detectDatastore(QString& deps, QString& depsFile) {
    if (deps.trimmed().toLower().contains("postgres")) { /* postgresql specific configuration */
        logInfo() << "Detected Postgresql dependency in file:" << depsFile;
        return Postgresql;
    }
    if (deps.trimmed().toLower().contains("mysql")) {
        logInfo() << "Detected Mysql dependency in file:" << depsFile;
        return Mysql;
    }

    logWarn() << "Falling back to SqLite3 driver cause no database defined in dependencies";
    return NoDB;
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


void createEnvironmentFiles(QString& serviceName, QString& domain, QString& stage, QString& branch) {

    logInfo() << "Creating app environment";
    QString servicePath = getServiceDataDir(serviceName);
    QString domainFilePath = servicePath + DEFAULT_SERVICE_DOMAIN_FILE;
    logDebug() << "Writing domain:" << domain << "to file:" << domainFilePath;
    writeToFile(domainFilePath, domain);

    auto latestRelease = readFileContents(servicePath + DEFAULT_SERVICE_LATEST_RELEASE_FILE).trimmed();
    logDebug() << "Current release:" << latestRelease;
    auto latestReleaseDir = servicePath + "/releases/" + latestRelease;
    logDebug() << "Release path:" << latestReleaseDir;
    auto appDetector = new WebAppTypeDetector(latestReleaseDir);
    auto appType = appDetector->getType();
    QString envEntriesString = "";
    logDebug() << "Detected application type:" << appDetector->typeName;
    delete appDetector;

    /* do app type specific action */
    auto clne = new SvdProcess("create_environment", getuid(), false);
    QStringList appDependencies;

    switch (appType) {

        case StaticSite: {

            QString jsonResult = "{\"alwaysOn\": false, \"watchPort\": false, ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            jsonResult += QString("\n\n\"start\": {\"commands\": \"echo 'Static app ready' >> SERVICE_PREFIX") + DEFAULT_SERVICE_LOG_FILE + " 2>&1 &" + "\"}\n}";
            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);
            logInfo() << "Launching service:" << serviceName;

            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case RubySite: {

            QString databaseName = serviceName + "-" + stage;
            WebDatastore database = NoDB;
            QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
            QString envFilePath = servicePath + DEFAULT_SERVICE_ENV_FILE;
            QString deps = "";

            if (QFile::exists(depsFile)) { /* NOTE: special software list file from Sofin, called ".dependencies" */
                deps = readFileContents(depsFile).trimmed();
            }
            database = detectDatastore(deps, depsFile);
            prepareSharedDirs(latestReleaseDir, servicePath, stage);
            generateDatastoreSetup(database, serviceName, stage, appType);

            /* write to service env file */
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "SSL_CERT_FILE=" + servicePath + DEFAULT_SSL_CA_FILE + "\n";
            envEntriesString += "RAILS_ENV=" + stage + "\n";
            envEntriesString += "RAKE_ENV=" + stage + "\n";
            writeToFile(envFilePath, envEntriesString);

            generateServicePorts(servicePath);

            /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
            appDependencies = deps.split("\n");
            logDebug() << "Gathering dependencies:" << appDependencies;
            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, ";
            QString environment = buildEnv(serviceName, appDependencies);
            logDebug() << "Generateed Service Environment:" << environment;
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            QString startResultJson = ""; /* command that actually launches main app */

            QMap<QString, QString> serviceWorkers; /* additional workers of service: (startCommands, stopCommands) */
            QString procFile = latestReleaseDir + "/Procfile"; /* heroku compatible procfile */
            if (QFile::exists(procFile)) {
                QStringList entries = readFileContents(procFile).trimmed().split("\n");
                logInfo() << "Proceeding with Procfile entries:" << entries;

                Q_FOREACH(QString entry, entries) {
                    QString procfileHead = entry.split(":").at(0);
                    QString procfileTail = entry.split(":").at(1);
                    QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
                    procfileTail = procfileTail.replace("$PORT", servPort); /* replace $PORT value of Procfile if exists */

                    if (procfileHead == "web") { /* web worker is defined here */
                        logInfo() << "Found web worker:" << procfileHead;
                        logDebug() << "Worker entry:" << procfileTail << "on port:" << servPort;
                        startResultJson += " cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " bundle exec " + procfileTail + " -b " + DEFAULT_LOCAL_ADDRESS + " -p " + servPort + " -P SERVICE_PREFIX" + DEFAULT_SERVICE_PID_FILE + " >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 & ";
                    } else {
                        logInfo() << "Found an entry:" << procfileHead;
                        QString procPidFile = procfileHead + ".pid";

                        serviceWorkers.insert( /* NOTE: by default, each worker must accept pid location, log location and daemon mode */

                            /* (start commands, stop commands) : */
                            "cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " bundle exec " + procfileTail + " -P " + servicePath + "/" + procPidFile + " -L " + servicePath + DEFAULT_SERVICE_LOG_FILE + "-" + procfileHead + " -d && \n echo 'Started worker " + procfileHead + "' >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ",

                            /* , stop commands) : */
                            "svddw $(cat " + servicePath + "/" + procPidFile + ") >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 "

                        );
                    }
                }

                /* generate correct order of application execution after workers */
                jsonResult += QString("\n\n\"start\": {\"commands\": \"");
                Q_FOREACH(QString part, serviceWorkers.keys()) { /* keys => start commands */
                    jsonResult += part + " &&\n";
                }
                jsonResult += startResultJson;
                jsonResult += "\"}";

                Q_FOREACH(QString acmd, serviceWorkers.keys()) {
                    QString cmd = serviceWorkers.take(acmd);
                    jsonResult += QString(", \n\n\"stop\": {\"commands\": \"");
                    jsonResult += cmd + " ";
                }
                jsonResult += "\"}\n}";

            } else { /* generate standard igniter entry */

                logInfo() << "Generating default entry (no Procfile used)";
                jsonResult += QString("\n\n\"start\": {\"commands\": \"") + "cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " bundle exec rails s -b " + DEFAULT_LOCAL_ADDRESS + " -p $(sofin port " + serviceName + ") -P SERVICE_PREFIX" + DEFAULT_SERVICE_PID_FILE + " >> SERVICE_PREFIX" + DEFAULT_SERVICE_LOG_FILE + " 2>&1 &" + "\"}\n}";
            }
            logDebug() << "Generated Igniter JSON:" << jsonResult;

            int steps = 0;
            while (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE)) {
                logDebug() << "Older service already running. Invoking stop for:" << serviceName;
                touch(servicePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);
                sleep(1);
                steps++;
                if (steps > OLD_SERVICE_SHUTDOWN_TIMEOUT) {
                    logError() << "Exitting endless loop, cause service:" << serviceName << "refuses to go down after " << QString::number(steps -1) << " seconds!";
                    break;
                }
            }

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            QString cacertLocation = QString(DEFAULT_CA_CERT_ROOT_SITE) + DEFAULT_SSL_CA_FILE;
            logInfo() << "Gathering SSL CA certs from:" << cacertLocation << "if necessary.";
            clne->spawnProcess("cd " + servicePath + " && test ! -f " + DEFAULT_SSL_CA_FILE + " && curl -C - -L -O " + cacertLocation + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1");
            clne->waitForFinished(-1);

            logInfo() << "Installing bundle for stage:" << stage << "of Rails Site";
            getOrCreateDir(servicePath + "/bundle-" + stage);
            clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle install --path " + servicePath + "/bundle-" + stage + " --without test development >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            prepareSharedSymlinks(latestReleaseDir, servicePath, stage);

            logInfo() << "Building assets";
            clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle exec rake assets:precompile >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            logInfo() << "Running database setup for database:" << getDbName(database);
            switch (database) {
                case Postgresql: {
                    logDebug() << "Creating user:" << databaseName;
                    clne->spawnProcess("createuser -s -d -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + getDbName(database) + " -p $(sofin port " + getDbName(database) + ") " + databaseName + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
                    clne->waitForFinished(-1);
                    logDebug() << "Creating database:" << databaseName;
                    clne->spawnProcess("createdb -h " + QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + getDbName(database) + " -p $(sofin port " + getDbName(database) + ") -O " + databaseName + " " + databaseName + " >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
                    clne->waitForFinished(-1);

                } break;

                default: break;

            }

            logInfo() << "Running database migrations";
            if (database != Postgresql) { /* postgresql db creation is already done before this hook */
                clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle exec rake db:create >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
                clne->waitForFinished(-1);
            }
            clne->spawnProcess("cd " + latestReleaseDir + " && " + buildEnv(serviceName, appDependencies) + " bundle exec rake db:migrate >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Relaunching service using newly generated igniter.";
            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case NodeSite: {

            prepareSharedDirs(latestReleaseDir, servicePath, stage);
            prepareSharedSymlinks(latestReleaseDir, servicePath, stage);

            generateServicePorts(servicePath, 2); /* XXX: 2 ports for node by default */

            /* generate env and write it to service.env file */
            QString servPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + DEFAULT_SERVICE_PORT_NUMBER).trimmed();
            QString websocketsPort = readFileContents(servicePath + DEFAULT_SERVICE_PORTS_DIR + "/1").trimmed(); // XXX: hardcoded
            logInfo() << "Building environment for stage:" << stage;
            envEntriesString += "LANG=" + QString(LOCALE) + "\n";
            envEntriesString += "NODE_ROOT=" + latestReleaseDir + "\n";
            envEntriesString += "NODE_ENV=" + stage + "\n";
            envEntriesString += "NODE_PORT=" + servPort + "\n";
            envEntriesString += "NODE_DOMAIN=" + domain + "\n";
            envEntriesString += "NODE_WEBSOCKET_PORT=" + websocketsPort + "\n";
            envEntriesString += "NODE_WEBSOCKET_CHANNEL_NAME=" + serviceName + "-" + domain + "\n";
            QString envFilePath = servicePath + DEFAULT_SERVICE_ENV_FILE;
            writeToFile(envFilePath, envEntriesString);

            QString depsFile = latestReleaseDir + SOFIN_DEPENDENCIES_FILE;
            QString deps = "", content = "";

            /* deal with dependencies. filter through them, don't add dependencies which shouldn't start standalone */
            if (QFile::exists(depsFile)) { /* NOTE: special software list file from Sofin */
                deps = readFileContents(depsFile).trimmed();
            }
            appDependencies = deps.split("\n");
            logDebug() << "Gathering dependencies:" << appDependencies;
            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"portsPool\": 2, ";
            QString environment = buildEnv(serviceName, appDependencies);
            logDebug() << "Generateed Service Environment:" << environment;
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            jsonResult += QString("\n\n\"start\": {\"commands\": \"") + "cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + "bin/app 2>&1 &" + "\"}\n}"; /* bin/app has to get all settings from ENV (stage in NODE_ENV) */
            logDebug() << "Generated Igniter JSON:" << jsonResult;

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE)) {
                logInfo() << "Older service already running. Invoking stop for:" << serviceName;
                touch(servicePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);
            }

            logInfo() << "Installing npm modules for stage:" << stage << "of Node Site";
            clne->spawnProcess("cd " + latestReleaseDir + " && \n" + buildEnv(serviceName, appDependencies) + " npm install >> " + servicePath + DEFAULT_SERVICE_LOG_FILE + " 2>&1 ");
            clne->waitForFinished(-1);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Relaunching service using newly generated igniter.";
            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case PhpSite: {

            QString jsonResult = "{\"alwaysOn\": true, \"watchPort\": true, \"softwareName\": \"Php\", ";
            jsonResult += generateIgniterDepsBase(latestReleaseDir, serviceName, branch, domain);
            #ifdef __APPLE__
                logError() << "Apple PHP deployments aren't supported yet!";
                raise(SIGTERM);
            #endif

            generateServicePorts(servicePath);

            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE)) {
                logInfo() << "Older service already running. Invoking stop for:" << serviceName;
                touch(servicePath + STOP_WITHOUT_DEPS_TRIGGER_FILE);
            }

            jsonResult += QString("\n\n\"start\": {\"commands\": \"" + buildEnv(serviceName, appDependencies) + " SERVICE_ROOT/exports/php-fpm -c SERVICE_PREFIX/service.ini --fpm-config SERVICE_PREFIX/service.conf --pid SERVICE_PREFIX/service.pid -D && \n echo 'Php app ready' >> SERVICE_PREFIX") + DEFAULT_SERVICE_LOG_FILE + " 2>&1" + "\"}\n}";

            /* write igniter to user igniters */
            QString igniterFile = QString(getenv("HOME")) + DEFAULT_USER_IGNITERS_DIR + "/" + serviceName + DEFAULT_SOFTWARE_TEMPLATE_EXT;
            logInfo() << "Generating igniter:" << igniterFile;
            writeToFile(igniterFile, jsonResult);

            logInfo() << "Starting server application";
            QFile::remove(QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + serviceName + START_TRIGGER_FILE);
            touch(QString(getenv("HOME")) + SOFTWARE_DATA_DIR + "/" + serviceName + START_TRIGGER_FILE);

            logInfo() << "Setting up autostart of service:" << serviceName;
            touch(servicePath + AUTOSTART_TRIGGER_FILE);

            spawnBinBuild(latestReleaseDir, serviceName, servicePath, appDependencies);

            prepareHttpProxy(servicePath, appType, latestReleaseDir, domain, serviceName, stage);

            touch(servicePath + DEFAULT_SERVICE_CONFIGURED_FILE);

            logInfo() << "Launching service using newly generated igniter.";
            if (QFile::exists(servicePath + DEFAULT_SERVICE_RUNNING_FILE))
                touch(servicePath + RESTART_WITHOUT_DEPS_TRIGGER_FILE);
            else
                touch(servicePath + START_WITHOUT_DEPS_TRIGGER_FILE);

        } break;


        case NoType: {
            logError() << "No web application detected in service directory:" << servicePath;
            raise(SIGTERM);

        } break;
    }

    clne->deleteLater();
}
