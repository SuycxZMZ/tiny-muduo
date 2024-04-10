#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <string>
#include <memory>
#include <vector>

#include "noncopyable.h"
#include "eventloopthread.h"

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop * baseLoop, const std::string nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { m_numThreads = numThreads; }
    void start(const ThreadInitCallback & cb = ThreadInitCallback());

    // 轮询
    EventLoop * getNextLoop();
    // 只留接口
    EventLoop * getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return m_started; }
    const std::string & name() const { return m_name; }
private:
    // server里的 main_loop
    EventLoop * m_baseLoop; 

    std::string m_name;
    bool m_started;
    int m_numThreads;

    // 轮询下标
    int m_next;

    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop*> m_loops;
};

#endif