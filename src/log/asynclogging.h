#ifndef ASYNC_LOGGING_H
#define ASYNC_LOGGING_H

#include "noncopyable.h"
#include "muduothread.h"
#include "fixedbuffer.h"
#include "logstream.h"
#include "logfile.h"

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

class AsyncLogging
{
public:
    AsyncLogging(const std::string &basename,
                 off_t rollSize,
                 int flushInterval = 3);
    ~AsyncLogging()
    {
        if (running_)
        {
            stop();
        }
    }

    // 前端调用 append 写入日志
    void append(const char *logling, int len);

    void start()
    {
        running_ = true;
        thread_.start();
    }

    void stop()
    {
        running_ = false;
        cond_.notify_one();
        thread_.join();
    }

private:
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;

    void threadFunc();

    const int flushInterval_;      // 冲刷缓冲数据到文件的超时时间, 默认3秒
    std::atomic<bool> running_;    // 后端线程loop是否运行标志
    const std::string basename_;   // 日志文件基本名称
    const off_t rollSize_;         // 日志文件滚动大小
    muduoThread thread_;           // 后端写线程
    std::mutex mutex_;             // 互斥锁
    std::condition_variable cond_; // 条件变量

    BufferPtr currentBuffer_; // 当前缓冲
    BufferPtr nextBuffer_;    // 空闲缓冲
    BufferVector buffers_;    // 已满缓冲队列
};

#endif // ASYNC_LOGGING_H