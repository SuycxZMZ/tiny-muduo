#include "Logging.h"
#include "Date.h"
#include "TimeZone.h"
#include "CurrentThread.h"

namespace tinymuduo
{
namespace ThreadInfo
{
    __thread char t_errnobuf[512];
    __thread char t_time[64];
    __thread time_t t_lastSecond;
};

// helper class for known string length at compile time
class T {
   public:
    T(const char* str, unsigned len) : str_(str), len_(len) { assert(strlen(str) == len_); }

    const char* str_;
    const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v) {
    s.append(v.str_, v.len_);
    return s;
}

const char* getErrnoMsg(int savedErrno)
{
    return strerror_r(savedErrno, ThreadInfo::t_errnobuf, sizeof(ThreadInfo::t_errnobuf));
}

// 根据Level返回Level名字
const char* getLevelName[Logger::LogLevel::LEVEL_COUNT]
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

Logger::LogLevel initLogLevel()
{
    return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

static void defaultOutput(const char* data, int len)
{
    fwrite(data, len, sizeof(char), stdout);
}

static void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
TimeZone g_logTimeZone;

Logger::Impl::Impl(Logger::LogLevel level, int savedErrno, const char* file, int line)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file)
{
    // 输出流 -> time
    formatTime2();
    // 写入日志等级
    stream_ << GeneralTemplate(getLevelName[level], 6);
    if (savedErrno != 0)
    {
        stream_ << getErrnoMsg(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

// Timestamp::toString方法的思路，只不过这里需要输出到流
void Logger::Impl::formatTime()
{
    Timestamp now = Timestamp::now();
    time_t seconds = static_cast<time_t>(now.microSecondsSinceEpoch() / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(now.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond);

    struct tm *tm_time = localtime(&seconds);
    // 写入此线程存储的时间buf中
    snprintf(ThreadInfo::t_time, sizeof(ThreadInfo::t_time), "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    // 更新最后一次时间调用
    ThreadInfo::t_lastSecond = seconds;

    // muduo使用Fmt格式化整数，这里我们直接写入buf
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%06d ", microseconds);

    // 输出时间，附有微妙(之前是(buf, 6),少了一个空格)
    stream_ << GeneralTemplate(ThreadInfo::t_time, 17) << GeneralTemplate(buf, 7);
}

void Logger::Impl::formatTime2() {
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != ThreadInfo::t_lastSecond) {
        ThreadInfo::t_lastSecond = seconds;
        struct DateTime dt;
        if (g_logTimeZone.valid()) {
            dt = g_logTimeZone.toLocalTime(seconds);
        } else {
            dt = TimeZone::toUtcTime(seconds);
        }

        int len = snprintf(ThreadInfo::t_time, sizeof(ThreadInfo::t_time), "%4d%02d%02d %02d:%02d:%02d", dt.year,
                           dt.month, dt.day, dt.hour, dt.minute, dt.second);
        assert(len == 17);
        (void)len;
    }

    if (g_logTimeZone.valid()) {
        Fmt us(".%06d ", microseconds);
        assert(us.length() == 8);
        stream_ << T(ThreadInfo::t_time, 17) << T(us.data(), 8);
    } else {
        Fmt us(".%06dZ ", microseconds);
        assert(us.length() == 9);
        stream_ << T(ThreadInfo::t_time, 17) << T(us.data(), 9);
    }
}

void Logger::Impl::finish()
{
    stream_ << " - " << GeneralTemplate(basename_.data_, basename_.size_) 
            << ':' << line_ << ":"
            << GeneralTemplate(tinymuduo::CurrentThread::tidString(), tinymuduo::CurrentThread::tidStringLength()) << '\n';;
}

// level默认为INFO等级
Logger::Logger(const char* file, int line)  
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(const char* file, int line, Logger::LogLevel level)
    : impl_(level, 0, file, line)
{
}

// 可以打印调用函数
Logger::Logger(const char* file, int line, Logger::LogLevel level, const char* func)
  : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}


Logger::~Logger()
{
    impl_.finish();
    // 获取buffer(stream_.buffer_)
    const LogStream::Buffer& buf(stream().buffer());
    // 输出(默认向终端输出)
    g_output(buf.data(), buf.length());
    // FATAL情况终止程序
    if (impl_.level_ == FATAL)
    {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}

Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

} // namespace tinymuduo

