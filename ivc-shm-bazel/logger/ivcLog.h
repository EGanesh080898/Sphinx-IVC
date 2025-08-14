#ifndef IVC_LOGGER__H
#define IVC_LOGGER__H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


#define IVC_LOG_ENABLE

#define GBUFFER_LEN 1024

typedef enum {
    IVC_LOG_NONE,
    IVC_LOG_ENTEXT,
    IVC_LOG_TRACE,
    IVC_LOG_DEBUG,
    IVC_LOG_INFO,
    IVC_LOG_WARNING,
    IVC_LOG_ERROR,
    IVC_LOG_CRITICAL,
    IVC_LOG_FATAL
} IvcLogLevel;

#ifdef IVC_LOG_ENABLE
#define IVC_LOG_ENTEXT(format, ...) Ivc_Log(IVC_LOG_ENTEXT, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_TRACE(format, ...) Ivc_Log(IVC_LOG_TRACE, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_DEBUG(format, ...) Ivc_Log(IVC_LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_INFO(format, ...) Ivc_Log(IVC_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_WARNING(format, ...) Ivc_Log(IVC_LOG_WARNING, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_ERROR(format, ...) Ivc_Log(IVC_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_CRITICAL(format, ...) Ivc_Log(IVC_LOG_CRITICAL, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define IVC_LOG_FATAL(format, ...) Ivc_Log(IVC_LOG_FATAL, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#else
#define IVC_LOG_ENTEXT(format, ...)
#define IVC_LOG_TRACE(format, ...)
#define IVC_LOG_DEBUG(format, ...)
#define IVC_LOG_INFO(format, ...)
#define IVC_LOG_WARNING(format, ...)
#define IVC_LOG_ERROR(format, ...)
#define IVC_LOG_CRITICAL(format, ...)
#define IVC_LOG_FATAL(format, ...)
#endif

static const char* LogStr[] = {"[ENTEXT] ", "[TRACE] ", "[DEBUG] ", "[INFO] ", "[WARNING] ", "[ERROR] ", "[CRITICAL] ", "[FATAL] "};
static uint16_t gLogLevel = (uint16_t)(0xFF << IVC_LOG_ERROR);
static pthread_mutex_t outMutex;

static uint8_t *getTimeStampNow()
{
    static uint8_t buffer[512] = {0};
    int len = 0;
    struct timeval tv;
    struct tm *sysTime;

    gettimeofday(&tv, NULL);
    sysTime = localtime(&tv.tv_sec);

    buffer[strftime(buffer, sizeof(buffer), "%H:%M:%S", sysTime)] = '\0';
    len = strlen(buffer);
    len += sprintf(buffer+len, ":%ld", tv.tv_usec);
    buffer[len] = '\0';

    return buffer;
}

static void writeToOutput(uint8_t *log) {
    pthread_mutex_lock(&outMutex);

    uint8_t* timeStamp = getTimeStampNow();
    printf("%s %s\n", timeStamp, log);

    pthread_mutex_unlock(&outMutex);
}

static void setLogLevel(IvcLogLevel level) {
    if (level == gLogLevel) return;
    gLogLevel = level;
}

static void setIvcLogLevel(IvcLogLevel level)
{
    uint16_t eLogLevel = 0;

    if (level<IVC_LOG_NONE && level>IVC_LOG_FATAL) return;

    switch(level){
        case IVC_LOG_NONE:
            eLogLevel = 0;
            break;
        case IVC_LOG_ENTEXT:
            eLogLevel = (uint16_t)(0xFF << 1);
            break;
        case IVC_LOG_TRACE:
            eLogLevel = (uint16_t)(0xFF << 2);
            break;
        case IVC_LOG_DEBUG:
            eLogLevel = (uint16_t)(0xFF << 3);
            break;
        case IVC_LOG_INFO:
            eLogLevel = (uint16_t)(0xFF << 4);
            break;
        case IVC_LOG_WARNING:
            eLogLevel = (uint16_t)(0xFF << 5);
            break;
        case IVC_LOG_ERROR:
            eLogLevel = (uint16_t)(0xFF << 6);
            break;
        case IVC_LOG_CRITICAL:
            eLogLevel = (uint16_t)(0xFF << 7);
            break;
        case IVC_LOG_FATAL:
            eLogLevel = (uint16_t)(0xFF << 8);
            break;
        default:
            break;
    }

   setLogLevel(eLogLevel);
}

static void Ivc_Log(IvcLogLevel level, const char *file, const char *function, int line, const char* format, ...)
{
    int32_t len = 0;
    uint8_t buffer[GBUFFER_LEN] = {0};

    if (level <= 0 || level > IVC_LOG_FATAL || gLogLevel == 0 || (gLogLevel & (1 << level)) == 0) {
        return;
    }
    // Initialize variable argument list
    va_list args;
    va_start(args, format);

    // Format the log message using a std::ostringstream
    sprintf(buffer, "%s:%d:%s:  ", file, line, function);
    len = strlen(buffer);
    vsnprintf(buffer+len, sizeof(buffer)-len, format, args);
    va_end(args);

    writeToOutput(buffer);
}

#endif
