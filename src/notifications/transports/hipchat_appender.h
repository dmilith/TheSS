/**
 *  @author dmilith
 *
 *   © 2013 - VerKnowSys
 *
 */


#ifndef _HIPCHAT_APPENDER_
#define _HIPCHAT_APPENDER_


#include "../../cutelogger/CuteLogger_global.h"
#include "../../cutelogger/AbstractStringAppender.h"


class CUTELOGGERSHARED_EXPORT HipChatAppender : public AbstractStringAppender {

    protected:
        virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                        const char* function, const QString& message);
};

#endif
