/**
 *  @author tallica
 *
 *   © 2013 - VerKnowSys
 *
 */

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "../globals/globals.h"
#include "../core/utils.h"

#include <unistd.h>
#include <QProcess>


class SvdProcess: public QProcess {

    public:
        SvdProcess(const QString& name);
        SvdProcess(const QString& name, bool redirectOutput);
        void spawnProcess(const QString& command, const QString& shell, const QStringList& args = QStringList("-s"));
        QString outputFile;

    protected:
        void setupChildProcess();

    private:
        bool redirectOutput = true;
        void init(const QString& name);

};


#endif
