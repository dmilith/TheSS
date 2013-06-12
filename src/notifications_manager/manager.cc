/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#include <QtCore>

#include "../globals/globals.h"
#include "../notifications/notifications.h"
#include "../service_spawner/utils.h"


int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName(DEFAULT_STRING_CODEC));
    QStringList args = app.arguments();

    /* Logger setup */
    ConsoleAppender *consoleAppender = new ConsoleAppender();
    Logger::registerAppender(consoleAppender);
    consoleAppender->setDetailsLevel(Logger::Fatal);

    logInfo() << "Notifications manager v0";


    return app.exec();
}
