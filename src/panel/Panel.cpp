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
}

void Panel::refreshServicesList() {
  QDir dir(home.absolutePath() + "/" + QString(SOFTWARE_DATA_DIR));
  QList<QFileInfo> list = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoDot);
  services.clear();
  Q_FOREACH(auto f, list){
    services << PanelService(f);
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
  QString aPid = QString(readFileContents(ssPidFile).c_str()).trimmed();
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
  return status;
}
