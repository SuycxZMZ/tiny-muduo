#include "EventLoopThreadPool.h"

namespace tinymuduo
{
EventLoopThreadPool::EventLoopThreadPool(EventLoop * baseLoop, const std::string nameArg) :
    m_baseLoop(baseLoop),
    m_name(nameArg),
    m_started(false),
    m_numThreads(0),
    m_next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // main_loop 起来的新线程在栈上创建，不用关心 m_loops 的析构
}

void EventLoopThreadPool::start(const ThreadInitCallback & cb)
{
    m_started = true;
    for (int i = 0; i < m_numThreads; ++i) {
        std::string buf = m_name;
        buf.resize(buf.size() + 32);
        // char buf[m_name.size() + 32] = {0};
        snprintf(&buf[0], buf.size(), "%s%d", m_name.c_str(), i);
        EventLoopThread * t = new EventLoopThread(cb, std::string(buf));
        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
        m_loops.emplace_back(t->startLoop());
    }
    if (m_numThreads == 0 && cb)
    {
        cb(m_baseLoop);
    }
}

EventLoop * EventLoopThreadPool::getNextLoop()
{
    EventLoop * loop = m_baseLoop;
    if (!m_loops.empty())
    {
        loop = m_loops[m_next];
        ++m_next;
        if (m_next >= m_loops.size())
        {
            m_next = 0;
        }
    }
    return loop;
}

EventLoop * EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    return nullptr;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if (m_loops.empty())
    {
        return {m_baseLoop};
    }
    else
    {
        return m_loops;
    }
}
} // namespace tinymuduo

