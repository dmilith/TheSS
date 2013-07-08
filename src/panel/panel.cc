/**
 *  @author dmilith, teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "panel.h"

Panel::Panel(QString user, QDir home, QDir ignitersDir):
    home(home), user(user), ignitersDir(ignitersDir) {

    getOrCreateDir(home.path());
    getOrCreateDir(ignitersDir.path());

    eventsManager = new SvdFileEventsManager();
    eventsManager->registerFile(home.path() + SOFTWARE_DATA_DIR);

    connect(eventsManager, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    // connect(eventsManager, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));

}

void Panel::setGui(PanelGui * gui){
    this->gui = gui;
    connect(this, SIGNAL(refreshed()), gui, SLOT(display()));
}

void Panel::onDirectoryChanged(QString dir){
    Q_UNUSED(dir);
    refresh();
}

void Panel::refresh(){
    refreshMutex.lock();
    // logDebug() << "refershing";
    refreshServicesList();
    refreshMutex.unlock();
    emit refreshed();
}

void Panel::refreshServicesList() {
    QDir dir(home.absolutePath() + "/" + QString(SOFTWARE_DATA_DIR));
    QList<QFileInfo> list = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoDot);

    Q_FOREACH(auto f, list){
        bool found = false;
        Q_FOREACH(auto s, services){
            if(s->baseDir == f){
                found = true;
                break;
            }
        }

        if(!found){
            logDebug() << "Found new service" << f.absoluteFilePath();
            services << new PanelService(this, f);
        }
    }

    Q_FOREACH(auto s, services){
        bool found = false;
        Q_FOREACH(auto f, list){
            if(s->baseDir == f){
                found = true;
                break;
            }
        }

        if(!found){
            logDebug() << "Found removed service" << s->baseDir.absoluteFilePath();
            services.removeOne(s);
        }
    }

    Q_FOREACH(auto s, services){
        s->refresh();
    }
}

QStringList * Panel::availableServices() {
    auto userEntries = QDir(home.absolutePath() + QString(DEFAULTUSERIGNITERSDIR)).entryList(QDir::Files);
    auto standardEntries = QDir(QString(DEFAULTSOFTWARETEMPLATESDIR)).entryList(QDir::Files);
    auto rootEntries = QStringList();
    if (getuid() == 0) {
            rootEntries << QDir(QString(SYSTEM_USERS_DIR) + QString(DEFAULTUSERIGNITERSDIR)).entryList(QDir::Files);
    }

    available.clear();
    available.append(userEntries);
    available.append(standardEntries);
    available.append(rootEntries);
    available.removeDuplicates();
    available.sort();
    available.replaceInStrings(QRegExp("\\.json"), "");
    return &available;
}

bool Panel::isSSOnline(){
    QString ssPidFile = home.absoluteFilePath("." + user + ".pid");
    QString aPid = readFileContents(ssPidFile).trimmed();
    bool ok = false;
    uint pid = aPid.toInt(&ok, 10);

    return (ok && pidIsAlive(pid));
}

void Panel::setLogLevel(QString level){
    touch(home.absoluteFilePath("." + level));
}

void Panel::shutdown(){
    touch(home.absoluteFilePath(".shutdown"));
}

QString Panel::addService(QString name){
    QString status;
    QDir dir(home.absolutePath() + "/" + QString(SOFTWARE_DATA_DIR) + "/" + name);

    if(!dir.exists()){ // service isn't already initialized
            auto all = availableServices();

            if (all->contains(name)) { /* is available */
                    QDir().mkpath(dir.absolutePath()); /* NOTE: the only thing required is to make directory in ~/SoftwareData/name */
                    status = "Initialized service: " + name;
            } else {
                    status = "Not found service igniter called: " + name;
            }

    } else {
            status = "Already defined service called: " + name;
    }

    refresh();
    return status;
}
