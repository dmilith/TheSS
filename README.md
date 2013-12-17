
## Authors:
* Daniel (dmilith) Dettlaff (dmilith [at] verknowsys.com). I'm also on #verknowsys @ freenode IRC.


## Contributors:
* Tymon (teamon) Tobolski.
* Michał (tallica) Lipski.


## Dependencies:
* [Sofin](http://verknowsys.github.io/sofin). (all igniters by default will try to use Sofin to install software)
* [Qt4 4.8.x](http://qt-project.org/downloads) (only QtCore part)


## Features
* Stateless, event driven, multithreaded and immutable inside (probably except uptime count state).
* Supports software igniters written in JSON (+comments support) to generate predefined software configurations on the fly.
* Supports software hooks. (More in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json)). Hook example:

```json
{
    "install": {
        "commands": "sofin install mysoft; install my software plugin",
        "expectOutput": "Success happened, and this output is a confirmation",
        "expectOutputTimeout": 0
    }
}
```

* Supports ordered, synchronous execution of service hooks (exception: scheduler commmands and http checks are executed asynchronously)
* Service hooks have predefined execution order and some will call others. This is built in behavior and it's constant:

```sh

startHook # will execute:
# ifNotInstalled(installHook) -> ifNotConfigured(configureHook) -> validateHook -> startHook -> afterStartHook

afterStartHook # will execute:
# afterStartHook

stopHook # will execute:
# stopHook -> afterStopHook

afterStopHook # will execute:
# afterStopHook

restartHook # will execute:
# stopHook -> afterStopHook -> ifNotConfigured(configureHook) -> validateHook -> startHook -> afterStartHook

validateHook # will execute:
# validateHook

reloadHook # will execute:
# reloadHook

installHook # will execute:
# installHook

configureHook # will execute:
# configureHook

reconfigureHook # will execute:
# configureHook -> restartHook

```

* Using kernel file watchers (kqueue on OSX & BSD, epoll on Linux) to watch software config/ data dirs.
* Supports "touch .hookname" method of manual launching of hooks. (for example: touch ~/SoftwareData/MyApp/.install will invoke install hook of MyApp igniter definition.)
* Supports basic hook dependency model. (for example: touch ~/SoftwareData/MyApp/.start will invoke: install hook, configure hook, validate hook and start hook of MyApp igniter definition)
* Supports software hook expectations with error reporting through built in Notification mechanism.
* Supports software "baby sitting". By default watches for software pid ("alwaysOn" option in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json))
* Supports TCP/ UDP port checking. ("watchPort" and "watchUdpPort" options in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json))
* Supports free port checking for TCP services, automatically generates random port for new services, (but also supports "staticPort" option in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json)), to provide static port binding for software. (for example staticPort is 80 for Nginx). By default, service ports are stored in ~/SoftwareData/AppName/.ports and reused on next software start.
* Supports service autostarting. You need only to touch ~/SoftwareData/MyApp/.autostart file.
* Is designed to work with user privileges as a little user side daemon.
* Uses almost no memory: ~5 MiB RSS on 64bit system when working with several services.
* Supports super user mode, to support root services (ports: 1-1024). Just run svdss as root and that's it.
* Supports state files. For example if process is running the lock file is created: ~/SoftwareData/MyApp/.running
* Supports unified process configuration model. By default ~/SoftwareData/MyApp/service.pid and ~/SoftwareData/MyApp/service.log are created for each software. If service requires configuration, it's by default stored in ~/SoftwareData/MyApp/service.conf
* Supports dynamic, live log level change invoked by touch ~/.log-level, where "log-level" is one of: error, info, debug, trace. By default log level is info.
* Supports auto igniter reload support on hook level. You don't need to update, sync or reload igniters. After change, next hook invoke will use latest version of software igniter by default.
* Supports multiple directory sources for igniters. Default order of checking igniters existance for regular user is: ~/Igniters/Services, then /SystemUsers/Igniters/Services. For root it's: /SystemUsers/Igniters/Services.
* Supports igniter constants, auto filled before invoke of each hook. Currently there are:

```sh
SERVICE_PREFIX            # by default: ~/SoftwareData/AppName
PARENT_SERVICE_PREFIX     # defines prefix of service parent
SERVICE_DOMAIN            # default host domain name. It's stored in ~/SoftwareData/AppName/.domain file by default.
SERVICE_ADDRESS           # by default it's default host IP address (resolved from SERVICE_DOMAIN)
SERVICE_ROOT              # by default: ~/Apps/AppName
SERVICE_VERSION           # by default taken from Sofin's: ~/Apps/AppName/appname.version

# You can also inject hook commands using igniter constants:
SERVICE_INSTALL_HOOK
SERVICE_START_HOOK
SERVICE_STOP_HOOK
SERVICE_AFTERSTART_HOOK
SERVICE_AFTERSTOP_HOOK
SERVICE_CONFIGURE_HOOK
SERVICE_BABYSITTER_HOOK
SERVICE_VALIDATE_HOOK
```

* Supports cron-compliant scheduler built in (since v0.24.x). Commands defined in schedulers have full support for igniter constants (listed above) and each command is in Zsh-compatible script format.
Scheduler example based on Redis igniter:

```json
{
    "schedulerActions": [
        {
            "cronEntry": "*/25 10-15 1,3,5,7 * * ?",
            "commands": "test -e SERVICE_PREFIX/database/database.rdf && cp SERVICE_PREFIX/database/database.rdf SERVICE_PREFIX/database/database.rdf-$(date +'%Y-%m-%d--%H%M').backup"
        },
        {
            "cronEntry": "*/10 * * * * ?",
            "commands": "for i in $HOME/triggers/*; do echo Invoking my magic trigger $i; echo $i; done"
        }
    ]
}
```

```sh
# Standard formatting for "cronEntry":
"*/10 10,11,12 1-15 * * ?"  # invoke each 10 minutes, exactly at 10am or 11am or 12am,
# only in first 15 days of month.
# The "?" sign is just a required symbolic placeholder which must be last character of each cron entry.

# Currently supported cron formats:
*       # WILDCARD: passes on each value
X       # NORMAL: passes when X has exact value as current value (X is a positive number)
*/X     # PERIODIC: passes when modulo of X and current value is 0 (X is a positive number)
X-Y     # RANGE: passes when value is in between X and Y (X, Y are positive numbers)
X,Y,Z   # SEQUENCE: passes when value is exactly one of X or Y or Z (X, Y, Z are positive numbers)

```

* Supports live updates. Do `touch ~/.shutdown`. Maintainer will go down but all your software services will stay in background. Then just do an upgrade, rebuild, install and run svdss again. It will just resume watch on live services. After a while (default babysitter pause time) it will automatically rebuild software dependency tree and continue to watch orphaned services.
* Support maintained, synchronous startup and shutdown of services. SIGINT (Ctrl - c in terminal) will shutdown TheSS gracefully including all running services. SIGTERM will just tell TheSS to shutdown leaving all services spawned in background (equivalent of `touch ~/.shutdown`).
* Supports basic UI - `svdpanel` - for easy service managment (based on ncurses library).
* Supports validation failure check. If validation fails startSlot won't be called. To set failure state, set `touch SERVICE_PREFIX/.validationFailure` in hook commands.
* Supports asynchronous http/https request-response check, for any number of urls (since 0.62.0).
* Supports Web Applications deployment - more about it below (since 0.66.x).
* Supports Procfile (Heroku-compatible) process list for RubySite deployment. If no Procfile detected in deployed app, default will be used.


## Igniter examples:
* [Redis](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Services/Redis.json)
* [Mysql](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Services/Mysql.json)
* [Postgresql](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Services/Postgresql.json)
* [Nginx](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/RootServices/Coreginx.json)


## But how to run it?

```sh
sofin install thess
# to install it for your user.

svdpanel
# to launch svdss in background by pressing F10 (if it's not already running)


# or

# Do everything manually:
bin/build

# to build and run tests:
bin/test

# and now just run it in foreground:
bin/svdss -f

# to actually spawn something with running thess in background, on second terminal, do:
mkdir ~/SoftwareData/Redis && touch ~/SoftwareData/Redis/.start # (this is an example of how to spawn Redis without using panel)

# wait a while, software installation process should work in background.
# This process should pick random port, generate configuration for Redis,
# and log output to file: ~/SoftwareData/Redis/service.log.
# Service starts it in background after that.

# you may also use built in panel:
bin/svdpanel
# then launch svdss in background by pressing F10 (if it's not already running)
# in panel, hit F7, type "Red", hit Enter, hit "S", and you just did the same as descibed above.
```



## Web deployer, and assumptions
TheSS provides script called `svdply`. It's designed to be launched on developer machine from local git repository.
It automatically performs deploy of supported web application from current directory, to machine (accessible
through ssh) on specified domain. First part of subdomain will be app name shown by `svdpanel` on remote side.
On first deploy `svdply` will create bare repository from current directory, and send it to remote
directory to: ~/Repos/myapp.git, hence current directory, must be a git repository (no other SCMs supported for now).


### Example deployer invocation:

```sh
 svdply myapp.mydomain.com
#|      |     |
#|      |      \____ "domain" part, used to:
#|      |            - locate host server address
#|       \           - do remote ssh connection
#|        \
#|         \________ "name" part, used in:
#|                   - igniter name (~/Igniters/myapp.json)
# \                  - git repository push address
#  \                   (by dafault: myapp.mydomain.com:Repos/myapp.git)
#  \                 - git repository name (on remote)
#   \
#    \______________ will:
#                    - do ssh connection to $USER@myapp.mydomain.com host
#                    - if "svd" remote in current git repository doesn't
#                      exists, will be created both on local and remote hosts
#                    - current branch of git repository (from current
#                      directory) will be pushed to "svd" remote
#                    - svddeployer will be launched on remote machine, and
#                      do the rest of web-app deploy
#

# params accepted by `svdply`:
APP_STAGE       # default: "staging"
APP_BRANCH      # default: "current_branch"
APP_USER        # default: $USER used by ssh to connect to remote host
APP_DOMAIN      # default: domain given as svdply param

# examples:
APP_STAGE=development svdply myapp.mydomain.com
APP_STAGE=production APP_BRANCH=production svdply myapp.mydomain.com


# then go to:
curl http://myapp.mydomain.com

# and if Web App was properly configured it will just happily reply on request :)
```

Please note that, you may use svdpanel to monitor your app with your dependencies on remote side to watch deployment progress.
Here're some assummptions for certain kinds of web-applications:

### For All web-apps:
* By default deploy process invokes `sofin dependencies` command which takes `.dependencies` file from repository root dir to determine all required software for web app (to be built properly). If file isn't detected, nothing wrong happens. For instance if you like to deploy a static web application, you may don't have `.dependencies` file at all.
* Following shell environment values are passed for each web-app:

```sh
LANG                        # UTF8 value of main locale that app should use.
                            # By default it's "en_GB.UTF-8"
```

### For Nodejs web-apps:
* By default you might want to have `node` entry in your `.dependencies` file.
* Following shell environment values are passed to web-app:

```sh
NODE_ROOT                   # absolute path to root directory of web-app
NODE_PORT                   # value contains generated port on which web-app
                            # main worker should listen on
NODE_ENV                    # value of environment stage
NODE_DOMAIN                 # value of domain name - what user did specify
                            # by doing "svdply domain.name"
NODE_WEBSOCKET_PORT         # value of additional port frequently used for
                            # websockets (this env might dissapear in future,
                            # cause it might be done automagically)
NODE_WEBSOCKET_CHANNEL_NAME # value of channel name of websockets root

# all of these values are accessible through process.env.VALUE_NAME on web-app side
```

### For Rails web-apps
* By default you might want to have at least `ruby` and `node` entries in your `.dependencies` file.
* Following shell environment values are passed to web-app:

```sh
SSL_CERT_FILE               # absolute path to cacerts file - generated
                            # and handled automatically by web-app
                            # deployment process
RAILS_ENV                   # value of environment stage of Rails
RAKE_ENV                    # value of environment stage of Rake
```


## Conflicts/ Problems/ Known issues:
* On OSX hosts, there's a very low default limit of maximum opened files (256). If user defined more than 10 services with such default, svdpanel may become unstable. It's recommended to increase this value to 2048 by `ulimit -n 2048`.
* Sofin build mechanism is known to be in conflict with other software managment solutions like: BSD Ports, HomeBrew, MacPorts, Fink. Keep that in mind before reporting problems.
* It's recommended to change shell by doing: `chsh -s /Software/Zsh/exports/zsh` for each user which will use TheSS. TheSS internally uses Shell from that path by default (currently Shell path is hardcoded due to lack of TheSS config in current version)


## FAQ
* "I don't want to use Sofin to install my software" - Thess is by definition designed to make use of Sofin software. If you want to use thess with some other utility - not Sofin - you're on your own. I'm not interested in developing any additional package managment support ever. It's that simple.
* "It doesn't work at all. No services are starting" - First, make sure you have working "svdss" in background. Then make sure you have default shell installed for launched services (/Software/Zsh). If you still have problems, please report.
* "I've found a SIGSEGV in your crappy code. How to help you track it?" - First of all, disable optimizations in src/Common.pro. Replace "-O3" with "-O0 -g" in compiler flags setting. Rebuild project from scratch. Then just run your code with "lldb" or "gdb", and when you reproduce an error please send me output of "bt" command. Your contibution is never forgotten!


## Used 3rd party software and licenses info:
* CuteLogger MT logger implementation by Boris Moiseev (LGPL licensed)
* Hiredis client library by Salvatore Sanfilippo and Pieter Noordhuis (BSD licensed)
* JSON CPP implementation with JSON comments support by Baptiste Lepilleur (MIT licensed)
* QuaZIP by Sergey A. Tachenov and contributors (LGPL licensed)
* QT4 4.8.x implementation by Trolltech & Nokia (LGPL licensed)
