#ifndef LOG_H
#define LOG_H

#include <string.h>

#include "timestamp.h"
#include "logstream.h"

namespace Log
{

class Logger 
{
public:
    enum LogLevel 
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char (&arr)[N]) :
            m_data(arr),
            m_size(N - 1)
        {
            const char * slash = strrchr(m_data, '/');
            if (slash)
            {
                m_data = slash + 1;
                m_size -= static_cast<int>(m_data - arr);
            }
        }

        explicit SourceFile (const char * filename) :
            m_data(filename)
        {
            const char * slash = strrchr(m_data, '/');
            if (slash)
            {
                m_data = slash + 1;
            }
            m_size = static_cast<int>(strlen(m_data));
        }

        const char* m_data;
        int m_size;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char * func);
    Logger(SourceFile file, int line, LogLevel level, bool toAbort);
    ~Logger();

    Log::LogStream& stream() { return m_impl.m_stream; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    using OutputFunc = void (*)(const char *, int);
    using FlushFunc = void (*)();
    static void setOutput(OutputFunc func);
    static void setFlush(FlushFunc func);
    static void setTimeZone(const Timestamp &time);
private:

class Impl
{
public:
    using logLevel = Log::Logger::LogLevel;
    Impl(logLevel level, int old_errno, const SourceFile & file, int line);
    void formatTime();
    void finish();

    Timestamp m_time;
    Log::LogStream m_stream;
    logLevel m_level;
    int m_line;
    SourceFile m_basename;
};
    Impl m_impl;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

#define LOG_TRACE if (Log::Logger::logLevel() <= Log::Logger::TRACE) \
    Log::Logger(__FILE__, __LINE__, Log::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Log::Logger::logLevel() <= Log::Logger::DEBUG) \
    Log::Logger(__FILE__, __LINE__, Log::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Log::Logger::logLevel() <= Log::Logger::INFO) \
    Log::Logger(__FILE__, __LINE__, Log::Logger::INFO, __func__).stream()
#define LOG_WARN Log::Logger(__FILE__, __LINE__, Log::Logger::WARN).stream()
#define LOG_ERROR Log::Logger(__FILE__, __LINE__, Log::Logger::ERROR).stream()
#define LOG_FATAL Log::Logger(__FILE__, __LINE__, Log::Logger::FATAL).stream()
#define LOG_SYSERR Log::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Log::Logger(__FILE__, __LINE__, true).stream()

const char * strerror_tl(int saveErrno);

#define CHECK_NOTNULL(val) \
    Log::CheckNotNull(__FILE__, __LINE__, "'"#val"' Must be non NULL", (val))

template<typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
    if (nullptr == ptr) 
    {
        Log::Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}

} // namespace Log



#endif // LOG_H