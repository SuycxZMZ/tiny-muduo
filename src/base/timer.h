#ifndef TIMER_H
#define TIMER_H

#include "noncopyable.h"
#include "timestamp.h"
#include <functional>

/**
 * Timer用于描述一个定时器
 * 定时器回调函数，下一次超时时刻，重复定时器的时间间隔等
 */
class Timer : noncopyable
{
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval)
        : m_callBack(move(cb)),
          m_expiration(when),
          m_interval(interval),
          m_repeat(interval > 0.0) // 一次性定时器设置为0
    {
    }

    void run() const 
    { 
        m_callBack(); 
    }

    Timestamp expiration() const  { return m_expiration; }
    bool repeat() const { return m_repeat; }

    // 重启定时器(如果是非重复事件则到期时间置为0)
    void restart(Timestamp now);

private:
    const TimerCallback m_callBack;  // 定时器回调函数
    Timestamp m_expiration;          // 下一次的超时时刻
    const double m_interval;         // 超时时间间隔，如果是一次性定时器，该值为0
    const bool m_repeat;             // 是否重复(false 表示是一次性定时器)
};

#endif // TIMER_H