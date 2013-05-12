
## Authors:
* Daniel (dmilith) Dettlaff (dmilith [at] verknowsys.com). I'm also on #freebsd and #scala.pl @ freenode IRC.


## Contributors:
* Michał (tallica) Lipski.


## Dependencies:
* [Sofin](http://verknowsys.github.io/sofin). (all igniters by default will try to use Sofin to install software)
* [Qt4 4.8.x](http://qt-project.org/downloads) (only QtCore part)


## Features
* Stateless, event driven, multithreaded and immutable inside (probably except uptime count state).
* Support software igniters written in JSON (+comments support) to generate predefined (user or system wide) software configurations on the fly.
* Support for software hooks. (More in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json)). Hook example:

```json
{
    "install": {
        "commands": "sofin install mysoft; install my software plugin",
        "expectOutput": "Success happened, and this output is a confirmation",
        "expectOutputTimeout": 0
    }
}
```

* Ordered, synchronous execution of service hooks (exception: scheduler commmands are executed asynchronously)
* Service hooks have predefined execution order and some will call others. This is built in behavior and it's constant.

```sh

startHook # will execute:
# ifNotInstalled(installHook -> configureHook) -> validateHook -> startHook -> afterStartHook

afterStartHook # will execute:
# afterStartHook

stopHook # will execute:
# stopHook -> afterStopHook

afterStopHook # will execute:
# afterStopHook

restartHook # will execute:
# stopHook -> afterStopHook -> validateHook -> startHook -> afterStartHook

validateHook # will execute:
# validateHook

reloadHook # will execute:
# reloadHook

installHook # will execute:
# installHook

configureHook # will execute:
# configureHook

```

* Using kernel file watchers (kqueue on OSX & BSD, epoll on Linux) to watch software config/ data dirs.
* Supports "touch .hookname" method of manual launching of hooks. (for example: touch ~/SoftwareData/MyApp/.install will invoke install hook of MyApp igniter definition.)
* Supports basic hook dependency model. (for example: touch ~/SoftwareData/MyApp/.start will invoke: install hook, configure hook, validate hook and start hook of MyApp igniter definition)
* Supports software hook expectations with error reporting through ~/SoftwareData/MyApp/.errors file.
* Supports software "baby sitting". By default watches for software pid ("alwaysOn" option in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json))
* Supports software TCP server checking. ("watchPort" option in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json))
* Supports free port checking for TCP services, automatically generates random port for new services, (but also supports "staticPort" option in [Defaults.json](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Default.json)), to provide static port binding for software. (for example staticPort is 80 for Nginx). By default, service ports are stored in ~/SoftwareData/AppName/.ports and reused on next software start.
* Supports service autostarting. You need only to touch ~/SoftwareData/MyApp/.autostart file.
* Is designed to work with user privileges as a little user side daemon.
* Uses almost no memory: ~5 MiB RSS on 64bit system when working with several services.
* Supports super user mode, to support root services (ports: 1-1024). Just run svdss as root and that's it.
* Should be system agnostic. I didn't try windows, but it should work after few minor changes.
* Supports state files. For example if process is running the lock file is created: ~/SoftwareData/MyApp/.running
* Supports unified process configuration model. By default ~/SoftwareData/MyApp/service.pid and ~/SoftwareData/MyApp/service.log are created for each software.
* Supports dynamic, live log level change invoked by touch ~/.log-level, where "log-level" is one of: error, info, debug, trace. By default log level is info.
* Supports auto igniter reload support on hook level. You don't need to update, sync or reload igniters. After change, next hook invoke will use latest version of software igniter by default.
* Supports multiple directory sources for igniters. Default order of checking igniters existance for regular user is: ~/Igniters/Services, then /Common/Igniters/Services. For root it's: /Common/Igniters/Services, then /SystemUsers/Igniters.
* Supports igniter constants, auto filled before invoke of each hook. Currently there are:

```sh
SERVICE_PREFIX            # by default: ~/SoftwareData/AppName
SERVICE_DEPENDENCY_PREFIX # defines prefix of service dependency
SERVICE_DOMAIN            # default host domain name. It's stored in ~/SoftwareData/AppName/.domain file by default.
SERVICE_ADDRESS           # by default it's default host IP address (resolved from SERVICE_DOMAIN)
SERVICE_ROOT              # by default: ~/Apps/AppName
SERVICE_VERSION           # by default taken from Sofin's: ~/Apps/AppName/appname.version
SERVICE_PORT              # by default: random port, stored in ~/SoftwareData/AppName/.ports
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

* Support for live updates. Do `touch ~/.shutdown` and all your software services will stay in background while maintainder will go down. Then just do an upgrade, rebuild, install and run again. It will just resume watch on live services.
* Support maintained, synchronous startup and shutdown of services. SIGINT (Ctrl - c in terminal) will shutdown TheSS gracefully including all running services. SIGTERM will just tell TheSS to shutdown leaving all services spawned in background (equivalent of `touch ~/.shutdown`).


## Igniter examples:
* [Redis](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Services/Redis.json)
* [Mysql](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/Services/Mysql.json)
* [Nginx](https://github.com/VerKnowSys/TheSS/blob/master/basesystem/universal/RootServices/Coreginx.json)


## WTF? How to run it?

```sh
# to install run as root:
bin/systemdeploy

# if you don't want to install it, but just try it,
# run as root:
bin/ignitersinstall # which installs default igniters to /Common

# to build and run tests:
bin/test && ./test

# to only build TheSS executables:
bin/build

# to install, run as root:
bin/systemdeploy

# and now just:
./svdss # or svdss if used install process
# (it supports -d param for explicit debug output and -t for trace level output
#  which may be later changed using "log level change" feature)

# on second terminal, do:
mkdir ~/SoftwareData/Redis && touch ~/SoftwareData/Redis/.start
# wait a while, software install process should work in background.
# This process has own log file here: ~/SoftwareData/Redis/.output.
# Software should pick random port, generate configuration for Redis
# service and just start it.
```


## Used 3rd party software and licenses info:
* CuteLogger MT logger implementation by Boris Moiseev (LGPL licensed)
* Hiredis client library by Salvatore Sanfilippo and Pieter Noordhuis (BSD licensed)
* JSON CPP implementation with JSON comments support by Baptiste Lepilleur (MIT licensed)
* QuaZIP by Sergey A. Tachenov and contributors (LGPL licensed)
* QT4 4.8.x implementation (LGPL licensed)
