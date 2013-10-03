
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
* Supports multiple directory sources for igniters. Default order of checking igniters existance for regular user is: ~/Igniters/Services, then /Common/Igniters/Services. For root it's: /Common/Igniters/Services, then /SystemUsers/Igniters.
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

* Support for live updates. Do `touch ~/.shutdown`. Maintainer will go down but all your software services will stay in background. Then just do an upgrade, rebuild, install and run maintainer again. It will just resume watch on live services. After a while (default babysitter pause time) it will automatically rebuild software dependency tree and continue to watch orphaned services.
* Support maintained, synchronous startup and shutdown of services. SIGINT (Ctrl - c in terminal) will shutdown TheSS gracefully including all running services. SIGTERM will just tell TheSS to shutdown leaving all services spawned in background (equivalent of `touch ~/.shutdown`).
* Supports basic UI - `svdpanel` - for easy service managment (based on ncurses library).
* Supports validation failure check. If validation fails startSlot won't be called. To set failure state, set `touch SERVICE_PREFIX/.validationFailure` in hook commands.


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
bin/test

# to only build TheSS executables:
bin/build

# to install, run as root:
bin/systemdeploy

# and now just:
bin/svdss # or svdss if used install process
# (it supports -d param for explicit debug output and -t for trace level output
#  which may be later changed using "log level change" feature)

# to actually spawn something, on second terminal, do:
mkdir ~/SoftwareData/Redis && touch ~/SoftwareData/Redis/.start
# wait a while, software install process should work in background.
# This process should log output to file: ~/SoftwareData/Redis/service.log,
# then should pick random port, generate configuration for Redis
# service and just start it.

# you may also use built in panel:
bin/panel
# Hit F7, type "Red", hit Enter, hit "S", and you just did the same as descibed above.
```


## Conflicts/ Problems/ Known issues:
* Sofin build mechanism is known to be in conflict with other software managment solutions like: BSD Ports, HomeBrew, MacPorts, Fink. Keep that in mind before reporting problems.
* It's recommended to change shell by doing: `chsh -s /Software/Zsh/exports/zsh` for each user which will use TheSS. TheSS internally uses Shell from that path by default (currently Shell path is hardcoded due to lack of TheSS config in current version)


## FAQ
* "It doesn't work at all. No services are starting" - First, make sure you have working "svdss" in background. Then make sure you have default shell installed for launched services (/Software/Zsh). If you still have problems, please report.
* "I've found a SIGSEGV in your crappy code. How to help you track it?" - First of all, disable optimizations in src/Common.pro. Replace "-O3" with "-O0 -g" in compiler flags setting. Rebuild project from scratch. Then just run your code with "lldb" or "gdb", and when you reproduce an error please send me output of "bt" command. Your contibution is never forgotten!


## Used 3rd party software and licenses info:
* CuteLogger MT logger implementation by Boris Moiseev (LGPL licensed)
* Hiredis client library by Salvatore Sanfilippo and Pieter Noordhuis (BSD licensed)
* JSON CPP implementation with JSON comments support by Baptiste Lepilleur (MIT licensed)
* QuaZIP by Sergey A. Tachenov and contributors (LGPL licensed)
* QT4 4.8.x implementation by Trolltech & Nokia (LGPL licensed)
