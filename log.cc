#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "log.h"
#include "currentthread.h"
#include "timestamp.h"

namespace Log
{

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char * strerror_tl(int saveErrno)
{
    return strerror_r(saveErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel()
{
    if (::getenv("MUDUO_LOG_TRACE"))
        return Logger::TRACE;
    else if (::getenv("MUDUO_LOG_DEBUG"))
        return Logger::DEBUG;
    else
        return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char * LogLevelName[Logger::NUM_LOG_LEVELS] = 
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
};

class T
{
public:
    T(const char * str, unsigned len) :
        m_str(str),
        m_len(len)
    {
        assert(strlen(str) == m_len);
    }

    const char * m_str;
    const unsigned m_len;
};

inline LogStream& operator<<(LogStream & s, T v)
{
    // code ...
    s.append(v.m_str, v.m_len);
    return s;
}

inline LogStream& operator<<(LogStream & s, const Logger::SourceFile & v)
{
    // code ...
    s.append(v.m_data, v.m_size);
    return s;
}

void defaultOutput(const char * msg, int len)
{
    size_t n = ::fwrite(msg, 1, len, stdout);
    //FIXME check n
    (void)n;
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
Timestamp g_logTime;

Logger::Logger(SourceFile file, int line) :
    m_impl(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level) :
    m_impl(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char * func) :
    m_impl(level, 0, file, line)
{
    m_impl.m_stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level, bool toAbort) :
    m_impl(toAbort ? FATAL : ERROR, errno, file, line)
{
}

Logger::~Logger()    
{
    m_impl.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (m_impl.m_level == FATAL)
    {
        g_flush();
        abort();
    }
}

Logger::Impl::Impl(logLevel level, int savedErrno, const SourceFile & file, int line) :
    m_time(Timestamp::now()),
    m_stream(),
    m_level(level),
    m_line(line),
    m_basename(file)
{
    formatTime();
    CurrentThread::tid();
    m_stream << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
    m_stream << T(LogLevelName[level], 6);
    if (0 != savedErrno)
    {
        m_stream << strerror_tl(savedErrno) << " (errno=" << savedErrno << ")";
    }
}

void Logger::Impl::formatTime()
{
    int len = snprintf(t_time, sizeof t_time, Timestamp::now().toString().c_str());
    (void)len;
    m_stream << T(t_time, len);
}

void Logger::Impl::finish()
{
    m_stream << " - " << m_basename << ':' << m_line << '\n';
}

void Logger::setLogLevel(LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc func)
{
    g_output = func;
}

void Logger::setFlush(FlushFunc func)
{
    g_flush = func;
}

void Logger::setTimeZone(const Timestamp &time)
{
    g_logTime = time;
}

} // namespace Log
