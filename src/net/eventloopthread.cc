#include "eventloopthread.h"


EventLoopThread::EventLoopThread(const ThreadInitCallBack & cb,
                                 const std::string & name) :
    m_loop(nullptr),
    m_exiting(false),
    m_thread(std::bind(&EventLoopThread::threadFunc, this), name),
    m_mutex(),
    m_cond_variable(),
    m_callback(cb)
{
}
EventLoopThread::~EventLoopThread()
{
    m_exiting = true;
    if (m_loop != nullptr)
    {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop * EventLoopThread::startLoop()
{
    assert(!m_thread.started());
    // 启动底层的新线程
    m_thread.start();

    EventLoop * loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // 子线程还没有把 m_loop 创建好
        while (m_loop == nullptr)
        {
            m_cond_variable.wait(lock);
        }
        loop = m_loop;
    }

    return loop;
}

// 在单独的新线程里执行
void EventLoopThread::threadFunc()
{
    // 和调用的线程一一对应， one loop per thread
    EventLoop loop;

    // EventLoopThread 创建时传入的回调
    if (m_callback)
    {
        m_callback(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond_variable.notify_one();
    }

    // poller.poll --> epoll_wait
    loop.loop();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}