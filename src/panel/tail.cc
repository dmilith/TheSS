/**
 *  @author teamon, dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "tail.h"

Tail::Tail(PanelService * service, const QString& dirname, const QString& filename):
        dirname(dirname), filename(filename), service(service){

    path = dirname + "/" + filename;

    eventsManager = new SvdFileEventsManager();
    eventsManager->registerFile(dirname);
    onDirectoryChanged(dirname);

    connect(eventsManager, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    connect(eventsManager, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));

    // QTimer *timer = new QTimer(this);
    // connect(timer, SIGNAL(timeout()), this, SLOT(readMoreLines()));
    // timer->start(100);

    // QTimer::singleShot(100, this, SLOT(readMoreLines()));

    connect(this, SIGNAL(updated()), service->panel->gui, SLOT(tailUpdate()));
    service->panel->gui->tailUpdate();
}

// Tail::~Tail(){
//     delete eventsManager;
//     eventsManager = NULL;
// }

void Tail::onDirectoryChanged(const QString& dir){
    if(dir != dirname || eventsManager == NULL) return;

    logDebug() << "onDirectoryChanged" << dir;

    if(QFile::exists(path)){
        if(!eventsManager->isWatchingFile(path)){
            eventsManager->registerFile(path);

            // get amount of rows of terminal window:
            struct winsize w;
            ioctl(0, TIOCGWINSZ, &w);

            // File created, seek to the end and read n lines from bottom
            readLinesBack(w.ws_row);
            onFileChanged(path);
        }
    } else {
        eventsManager->unregisterFile(path);
        frontPos = backPos = 0;
        buffer.push_back("** FILE REMOVED **");
    }

    logDebug() << "updated";
    emit updated();
}


void Tail::readMoreLines(){
    readLinesFront(120);
    onFileChanged(path);
}

void Tail::readLinesBack(int n){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        logError() << "Error reading file " << path;
        return;
    }

    qint64 size = file.size();
    QTextStream stream(&file);
    QList<QString> buf;
    qint64 block = 5;
    qint64 pos = -1;

    do {
        buf.clear();
        pos = size - (n*block);
        if(pos < 0) pos = 0;
        stream.seek(pos);

        if(pos != 0) stream.readLine(); // read one line to ensure we don't start reading in the middle of line
        while(!stream.atEnd()) buf.push_back(stream.readLine());

        block *= 2;
    } while(buf.size() < n && pos != 0);

    frontPos = pos;
    backPos = stream.pos();

    for(int i=0; i < buf.size(); i++) buffer.push_back(buf.at(i));
    file.close();

    logDebug() << "updated";
    emit updated();
}

void Tail::readLinesFront(int n){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        logError() << "Error reading file " << path;
        return;
    }

    QTextStream stream(&file);
    QList<QString> buf;
    qint64 block = 4;
    qint64 pos = -1;
    qint64 npos = -1;

    do {
        buf.clear();
        pos = frontPos - (n*block);
        if(pos < 0) pos = 0;
        stream.seek(pos);

        if(pos != 0) stream.readLine(); // read one line to ensure we don't start reading in the middle of line
        npos = stream.pos();

        while(stream.pos() < frontPos) buf.push_back(stream.readLine());

        block *= 2;
    } while(buf.size() < n && pos != 0);

    frontPos = npos;

    for(int i=buf.size()-1; i >= 0; i--) buffer.push_front(buf.at(i));
    file.close();

    logDebug() << "updated";
    emit updated();
}

void Tail::onFileChanged(const QString& p){
    if(p != path) return; // do not care about other files in directory

    logDebug() << "onFileChanged" << path;

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        logError() << "Error reading file " << path;
        return;
    }

    int newcount=0;

    QTextStream stream(&file);
    stream.seek(backPos);

    while(!stream.atEnd()){
        buffer.push_back(stream.readLine());
        newcount++;
    }

    if(newcount > 0 && scrollOffset > 0){
      newData = true;
      scrollOffset += newcount;
    }

    backPos = stream.pos();

    file.close();

    logDebug() << "updated";
    emit updated();
}

void Tail::resetScroll(){
  scrollOffset = 0;
  newData = false;
}

void Tail::scrollLog(int n, int rows){
  logDebug() << "scrollLog " << n << rows;

  scrollOffset += n;
  frontReached = false;
  if(scrollOffset < 0) scrollOffset = 0;
  if(buffer.size() - scrollOffset < rows - 1){
    readLinesFront(100);
    if(buffer.size() - scrollOffset <= rows - 1){
      frontReached = true;
      scrollOffset = max(0, buffer.size() - (rows - 1));
    }
  }

  if(scrollOffset == 0) newData = false;
}

void Tail::toggleWrap(){
  wrap = !wrap;
}

void Tail::display(WINDOW * win, int rows, int cols){
  int s = buffer.size() - scrollOffset;
  int off = max(s-rows, 0);
  int m = min(s, rows);
  int i=0;
  int w = cols/2 - 2;

  wmove(win, 0, 0);
  wvline(win, '|', rows);


  QList<QString> buf;

  logDebug() << "tail display" << "bufsize: " << buffer.size() << "off " << off << "m: " << m;

  for(i=0; i<m; i++){
      QString line = buffer.at(i+off);
      if(wrap){
        do {
            if(line.size() > w){
                buf.push_back(line.left(w-2) + " ⏎");
                line.replace(0, w-2, "");
            } else {
                buf.push_back(line.left(w));
                line.replace(0, w, "");
            }
        } while(line.size() > 0);
      } else {
        buf.push_back(line);
      }
  }

  s = buf.size();
  off = max(s-rows, 0);
  m = min(s, rows);

  for(i=0; i<m; i++){
      QString line = buf.at(i+off);
      mvwprintw(win, i, 0, "| %s", line.toUtf8().data());
      wclrtoeol(win);
  }

  for(;i<rows; i++){
      wmove(win, i, 2);
      wclrtoeol(win);
  }

  if(scrollOffset > 0){
    rows -= 1;
    wattron(win, C_SCROLLING);
    wbkgdset(win, C_SCROLLING);
    mvwprintw(win, rows, 1, " Scrolling (%d lines up)", scrollOffset);
    if(newData) wprintw(win, "  --  New data available!");
    if(frontReached) wprintw(win, "  --  Reached top of file");
    wclrtoeol(win);
    wattroff(win, C_SCROLLING);
    wbkgdset(win, 0);
  }
}
