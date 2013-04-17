/**
 *  @author tallica
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "process.h"


void SvdProcess::init(const QString& name, uid_t uid) {
    this->uid = uid;
    outputFile = getSoftwareDataDir(uid) + "/" + name + DEFAULT_SERVICE_OUTPUT_FILE;
    setProcessChannelMode(MergedChannels);

    if (QFile::exists(outputFile)) {
        logTrace() << "Rotating log file" << outputFile;
        rotateFile(outputFile);
    }
    if (this->redirectOutput)
        setStandardOutputFile(outputFile, QIODevice::Truncate);
}


SvdProcess::SvdProcess(const QString& name, uid_t uid, bool redirectOutput) {
    this->redirectOutput = redirectOutput;
    init(name, uid);
}


SvdProcess::SvdProcess(const QString& name, uid_t uid) {
    init(name, uid);
}


SvdProcess::SvdProcess(const QString& name) {
    init(name, getuid());
}


void SvdProcess::spawnDefaultShell() {
    logDebug() << "Spawning default shell.";
    start(QString(DEFAULT_SHELL_COMMAND), QStringList("-s"));
}


void SvdProcess::spawnProcess(const QString& command) {
    spawnDefaultShell();
    logTrace() << "Spawning command:" << QString(command);
    write(command.toUtf8());
    closeWriteChannel();
    // stop();
}


void SvdProcess::setupChildProcess() {
    const QString home = getHomeDir(uid);
    const QString user = QString::number(uid).toUtf8();
    logDebug() << "Setup process environment with home:" << home << "and user:" << user;

    #ifdef __FreeBSD__
        setgroups(0, 0);
    #endif
    setuid(uid);
    chdir(home.toUtf8());
    setenv("HOME", home.toUtf8(), 1);
    setenv("~", home.toUtf8(), 1);
    setenv("PWD", home.toUtf8(), 1);
    setenv("OLDPWD", home.toUtf8(), 1);
    setenv("USER", user.toUtf8(), 1);
    setenv("LOGNAME", user.toUtf8(), 1);
    setenv("LC_ALL", LOCALE, 1);
    setenv("LANG", LOCALE, 1);
    unsetenv("USERNAME");
    unsetenv("SUDO_USERNAME");
    unsetenv("SUDO_USER");
    unsetenv("SUDO_UID");
    unsetenv("SUDO_GID");
    unsetenv("SUDO_COMMAND");
    unsetenv("MAIL");
}
