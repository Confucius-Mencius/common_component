#include <cstdio>
#include <string>
#include <pthread.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>

using namespace std;

static log4cplus::Logger lgPltf = log4cplus::Logger::getInstance("loggerPlatform");

void* threadFun(void* arg)
{
    char name[64] = {0};
    snprintf(name, sizeof(name) - 1, "loggerMkt.%s", (const char*) arg);
    log4cplus::Logger loggerMkt1 = log4cplus::Logger::getInstance(name);
    //snprintf(name, sizeof(nameAppendFile)-1, "SQFront.%s.log", arg);
    //log4cplus::SharedAppenderPtr apd(new log4cplus::TimeBasedRollingFileAppender(nameAppendFile));
    //std::auto_ptr<log4cplus::Layout> _layout(new log4cplus::TTCCLayout());
    //apd->setLayout(_layout);
    //loggerCallback.addAppender(apd);

    LOG4CPLUS_TRACE_METHOD(loggerMkt1, LOG4CPLUS_TEXT(__FUNCTION__));
    LOG4CPLUS_DEBUG(loggerMkt1, "This is a DEBUG message");
    LOG4CPLUS_INFO(loggerMkt1, "This is a INFO message");
    LOG4CPLUS_WARN(loggerMkt1, "This is a WARN message");
    LOG4CPLUS_ERROR(loggerMkt1, "This is a ERROR message");
    LOG4CPLUS_FATAL(loggerMkt1, "This is a FATAL message");

    return NULL;
}

void printDebug()
{
    LOG4CPLUS_TRACE_METHOD(lgPltf, LOG4CPLUS_TEXT(__FUNCTION__));
    LOG4CPLUS_DEBUG(lgPltf, "This is a DEBUG message");
    LOG4CPLUS_INFO(lgPltf, "This is a INFO message");
    LOG4CPLUS_WARN(lgPltf, "This is a WARN message");
    LOG4CPLUS_ERROR(lgPltf, "This is a ERROR message");
    LOG4CPLUS_FATAL(lgPltf, "This is a FATAL message");
}

int main()
{
    pthread_t pthread1, pthread2;
    log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("SQFront.log.properties"));
    log4cplus::Logger loggerMktBase = log4cplus::Logger::getInstance("loggerMkt");
    pthread_create(&pthread1, NULL, threadFun, (void*) "tdf");
    pthread_create(&pthread2, NULL, threadFun, (void*) "gta");
    printDebug();
    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);

    return 0;
}