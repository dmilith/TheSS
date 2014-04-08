## Known Issues:
* Dispel part isn't production ready. It doesn't work for now.
* Deployer part isn't production ready. It works, but it's not stable yet.
* svdcoreginx_helper sucks, it's written badly and it doesn't reload Coreginx on first svdply deploy.
* LiveUsers igniter should be more intelligent. It now reloads ttys even if there were no changes in user autostart trigger.
* Panel is generating high CPU load (which is caused by live monitoring of each service files).
* Panel becomes unresponsive with extremely high amounts of logs written by service.
* Igniters file changes must be atomic.

## TODO list (in order of importance):
* Auto scaling support with intelligent round robin with host failure detection (full distributed architecture support). Difficulty approx.: 5/10.
* Implement software conflict check before run (error notification when f.e. Ruby is a requirement of service1 and Ruby19 is a requirement of service2). Difficulty approx.: 8/10.
* Add support for TheSS configuration (get rid of a couple of hardcodes, f.e.: Shell path). Difficulty approx.: 5/10.
* Internal/ External networks support (VPN/ Internet). Difficulty approx.: 7/10.
