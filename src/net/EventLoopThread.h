#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <condition_variable>
#include <string>
#include <assert.h>

#include "noncopyable.h"
#include "EventLoop.h"
#include "MuduoThread.h"

namespace tinymuduo
{
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallBack = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallBack & cb = ThreadInitCallBack(),
                    const std::string & name = std::string());
    ~EventLoopThread();

    EventLoop * startLoop();
private:
    void threadFunc();

    EventLoop * m_loop;
    bool m_exiting;
    muduoThread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond_variable;
    ThreadInitCallBack m_callback;
};
}


#endif