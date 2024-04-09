#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <functional>

#include "eventloop.h"
#include "logger.h"
#include "poller.h"
#include "channel.h"

// class Poller;
// 防止一个线程创建多个 event_loop
__thread EventLoop * t_loopInThisThread = 0;

// 默认的 poller 超时时间 10s
const int kPollTimeMs = 10000;

// 创建 wakeupfd，用来notify唤醒 subloop
int createEventfd()
{  
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("func = %s, create eventfd error", __FUNCTION__);
    }
    return evtfd;
}

EventLoop::EventLoop() :
    m_looping(false),
    m_quit(false),
    m_callingPendingFunctors(false),
    m_threadId(CurrentThread::tid()),
    m_poller(Poller::newDefaultPoller(this)),
    m_wakeupFd(createEventfd()),
    m_wakeupChannel(new Channel(this, m_wakeupFd)),
    m_currentActiveChannel(nullptr)
{
    LOG_DEBUG("EventLoop create at %p, in thread %d", this, m_threadId);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThisThread, m_threadId);
    }
    else
    {
        t_loopInThisThread = this;
    }
    m_wakeupChannel->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    // 每个loop都监听 wakeupfd 上的读事件，main_loop 负责在每个 sub_loop 的wakeupfd上写，唤醒sub_loop
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG("EventLoop %p of thread %d destructs in thread %d", this, m_threadId, CurrentThread::tid());
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeupFd);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(m_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8", n);
    }
}


void EventLoop::loop()
{
    m_looping = true;
    m_quit = false;
    LOG_INFO("EventLoop %p start looping", this);

    while (!m_quit)
    {
        m_channelList.clear();
        // main_loop 监听的是wakeupfd, sub_loop监听的是clientfd
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_channelList);
        for (Channel * channel : m_channelList)
        {
            m_currentActiveChannel = channel;
            m_currentActiveChannel->handelEvent(m_pollReturnTime);
        }
        m_currentActiveChannel = nullptr;
        // 执行当前Eventloop需要处理的回调操作
        /**
         * main_loop --> 只做 accept用，返回 fd --> 打包为channel --> 给sub_loop
         * main_loop 事先注册一个回调(sub_loop使用) sub_loop被唤醒之后执行
        */
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping", this);
    m_looping = false;
}

void EventLoop::quit()
{
    m_quit = true;
    /**
     * 在其他的线程中调用了quit 走下面的if 
     * 要先把对应 loop 唤醒，使其在 poll上返回，才能跳出 loop 中的while循环
     * 每个loop都可以通过 wakeupfd 来唤醒
    */
    if (!isInLoopThread())
    {
        wakeUp();
    }
}

// 在当前loop线程中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else // 在非当前loop的线程中执行cb, 要唤醒loop所在线程，执行cb
    {
        queueInLoop(std::move(cb));
    }
}

// 把cb放入队列，在其他线程执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingFunctors.emplace_back(std::move(cb));
    }
    // m_callingPendingFunctors ？ 
    if (!isInLoopThread() || m_callingPendingFunctors)
    {
        wakeUp();
    }
}

void EventLoop::wakeUp()
{
    uint64_t one = 1;
    ssize_t n = ::write(m_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes instead of 8", n);
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return m_poller->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }
    for (const Functor & functor : functors)
    {
        functor();
    }
    m_callingPendingFunctors = false;
}