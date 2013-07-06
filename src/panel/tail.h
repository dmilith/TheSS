/**
 *  @author teamon
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __TAIL_H__
#define __TAIL_H__

#include "panel.h"

class Tail : QObject {
    Q_OBJECT

public:
    Tail(PanelService * service, const QString& dirname, const QString& filename);
    void readLinesFront(int n);
    void readLinesBack(int n);
    QList<QString> buffer;
    void display(WINDOW * win, int rows, int cols);
    void scrollLog(int n, int rows);
    void toggleWrap();
    void resetScroll();

signals:
    void updated();

private slots:
    void onDirectoryChanged(const QString& dir);
    void onFileChanged(const QString& file);
    void readMoreLines();

private:
    SvdFileEventsManager * eventsManager;
    qint64 frontPos = 0;
    qint64 backPos = 0;
    qint64 lastSize = 0;
    QString dirname;
    QString filename;
    QString path;
    PanelService * service;
    int scrollOffset = 0;
    bool newData = false;
    bool wrap = true;
    bool frontReached = false;
    QMutex mutex;
};


#endif
