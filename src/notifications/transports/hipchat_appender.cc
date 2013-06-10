/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */

#include "hipchat.h"
#include "hipchat_appender.h"


void HipChatAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                             const char* function, const QString& message) {
    //std::cerr << qPrintable(formattedString(timeStamp, logLevel, file, line, function, message));
    notification(qPrintable(formattedString(timeStamp, logLevel, file, line, function, message)));
}
