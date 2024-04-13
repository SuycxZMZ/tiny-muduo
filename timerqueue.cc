#include "eventloop.h"
#include "channel.h"
#include "logger.h"
#include "timer.h"
#include "timerqueue.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>

int createTimerfd()
{
    /**
     * CLOCK_MONOTONIC：绝对时间
     * TFD_NONBLOCK：非阻塞
     */
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                    TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_ERROR("Failed in timerfd_create");
    }
    return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop)
    : m_loop(loop),
      m_timerfd(createTimerfd()),
      m_timerfdChannel(m_loop, m_timerfd),
      m_timers()
{
    m_timerfdChannel.setReadCallBack(
        std::bind(&TimerQueue::handleRead, this));
    m_timerfdChannel.enableReading();
}

TimerQueue::~TimerQueue()
{   
    m_timerfdChannel.disableAll();
    m_timerfdChannel.remove();
    ::close(m_timerfd);
    // 删除所有定时器
    for (const Entry& timer : m_timers)
    {
        delete timer.second;
    }
}

void TimerQueue::addTimer(TimerCallback cb,
                          Timestamp when,
                          double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    m_loop->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    // 是否取代了最早的定时触发时间
    bool eraliestChanged = insert(timer);

    // 我们需要重新设置m_timerfd触发时间
    if (eraliestChanged)
    {
        resetTimerfd(m_timerfd, timer->expiration());
    }
}

// 重置timerfd
void TimerQueue::resetTimerfd(int m_timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, '\0', sizeof(newValue));
    memset(&oldValue, '\0', sizeof(oldValue));

    // 超时时间 - 现在时间
    int64_t microSecondDif = expiration.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microSecondDif < 100)
    {
        microSecondDif = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microSecondDif / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microSecondDif % Timestamp::kMicroSecondsPerSecond) * 1000);
    newValue.it_value = ts;
    // 此函数会唤醒事件循环
    if (::timerfd_settime(m_timerfd, 0, &newValue, &oldValue))
    {
        LOG_ERROR("timerfd_settime faield()");
    }
}

void ReadTimerFd(int timerfd) 
{
    uint64_t read_byte;
    ssize_t readn = ::read(timerfd, &read_byte, sizeof(read_byte));
    
    if (readn != sizeof(read_byte)) 
    {
        LOG_ERROR("TimerQueue::ReadTimerFd read_size < 0");
    }
}

// 返回删除的定时器节点 （std::vector<Entry> expired）
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    // TODO:???
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = m_timers.lower_bound(sentry);
    std::copy(m_timers.begin(), end, back_inserter(expired));
    m_timers.erase(m_timers.begin(), end);
    
    return expired;
}

void TimerQueue::handleRead()
{
    Timestamp now = Timestamp::now();
    ReadTimerFd(m_timerfd);

    std::vector<Entry> expired = getExpired(now);

    // 遍历到期的定时器，调用回调函数
    m_callingExpiredTimers = true;
    for (const Entry& it : expired)
    {
        it.second->run();
    }
    m_callingExpiredTimers = false;
    
    // 重新设置这些定时器
    reset(expired, now);

}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;
    for (const Entry& it : expired)
    {
        // 重复任务则继续执行
        if (it.second->repeat())
        {
            auto timer = it.second;
            timer->restart(Timestamp::now());
            insert(timer);
        }
        else
        {
            delete it.second;
        }

        // 如果重新插入了定时器，需要继续重置timerfd
        if (!m_timers.empty())
        {
            resetTimerfd(m_timerfd, (m_timers.begin()->second)->expiration());
        }
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = m_timers.begin();
    if (it == m_timers.end() || when < it->first)
    {
        // 说明最早的定时器已经被替换了
        earliestChanged = true;
    }

    // 定时器管理红黑树插入此新定时器
    m_timers.insert(Entry(when, timer));

    return earliestChanged;
}

