#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

#include "noncopyable.h"
#include "timestamp.h"
#include "currentthread.h"
#include "logger.h"

class Channel;
class Poller;
// 时间循环类 -- > 包括 Channel 和 Poller 两个类
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop(); // 开启事件循环
    void quit();

    Timestamp pollReturnTime() const { return m_pollReturnTime; }

    void runInLoop(Functor cb);   // 在当前loop中执行cb
    void queueInLoop(Functor cb); // 把cb放入队列

    // 内部接口
    void wakeUp(); // 唤醒 loop 所在线程
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return m_threadId == CurrentThread::tid(); }

private:
    void handleRead();        // waked up
    void doPendingFunctors(); // 回调

    using ChannelList = std::vector<Channel *>;

    std::atomic_bool m_looping;
    std::atomic_bool m_quit;
    std::atomic_bool m_callingPendingFunctors; // 标识当前线程是否有需要回调的操作
    const pid_t m_threadId;                    // 当前loop所在线程的id
    Timestamp m_pollReturnTime;
    std::unique_ptr<Poller> m_poller;
    int m_wakeupFd;                           // 通过eventfd系统调用，mainloop拿到新的channel后，轮询选择一个subloop，通过该成员唤醒subloop
    std::unique_ptr<Channel> m_wakeupChannel; // 包含 m_wakeupFd 以及对应的事件

    ChannelList m_channelList;
    Channel *m_currentActiveChannel;

    std::mutex m_mutex;                     // 保护m_pendingFunctors的线程安全
    std::vector<Functor> m_pendingFunctors; // 存储loop需要执行的所有回调操作
};

EventLoop * CheckLoopNotNull(EventLoop * loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("main loop is nullptr");
    }
    return loop;
}

#endif