/**
 *  @author dmilith
 *
 *   © 2014 - VerKnowSys
 *
 */


#include "utils-core.h"


/*
 *  Read file contents of text file
 */
QString readFileContents(const QString& fileName) {
    QString lines = "";
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream stream(&f);
    // stream.setCodec(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (!line.trimmed().isEmpty()) {
            lines += line + "\n";
        }
    }
    lines += "\n";
    f.close();
    return lines;
}

