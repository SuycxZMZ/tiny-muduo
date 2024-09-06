#ifndef LOGGING_H
#define LOGGING_H

#include "TimeStamp.h"
#include "LogStream.h"

#include <sys/time.h>
#include <string.h>
#include <functional>

namespace tinymuduo
{
// SourceFile的作用是提取文件名
class SourceFile
{
public:
    explicit SourceFile(const char* filename)
        : data_(filename)
    {
        // 找出data中出现 "/" 最后一次的位置，从而获取具体的文件名
        const char* slash = strrchr(filename, '/');
        if (slash)
        {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
};

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
        LEVEL_COUNT,
    };

    // member function
    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    // 流是会改变的
    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    // 输出函数和刷新缓冲区函数
    using OutputFunc = std::function<void(const char* msg, int len)>;
    using FlushFunc = std::function<void()>;
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);

private:
    // 内部类
    class Impl
    {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int savedErrno, const char* file, int line);
        void formatTime();
        void formatTime2();
        void finish();

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };

    // Logger's member variable
    Impl impl_;
};

extern Logger::LogLevel g_logLevel;

// 获取errno信息
const char* getErrnoMsg(int savedErrno);


} // namespace tinymuduo

#define LOG_DEBUG if (tinymuduo::Logger::logLevel() <= tinymuduo::Logger::DEBUG) \
  tinymuduo::Logger(__FILE__, __LINE__, tinymuduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (tinymuduo::Logger::logLevel() <= tinymuduo::Logger::INFO) \
  tinymuduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN tinymuduo::Logger(__FILE__, __LINE__, tinymuduo::Logger::WARN).stream()
#define LOG_ERROR tinymuduo::Logger(__FILE__, __LINE__, tinymuduo::Logger::ERROR).stream()
#define LOG_FATAL tinymuduo::Logger(__FILE__, __LINE__, tinymuduo::Logger::FATAL).stream()

#endif // LOGGING_H