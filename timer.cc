#include "timer.h"

void Timer::restart(Timestamp now)
{
    if (m_repeat)
    {
        // 如果是重复定时事件，则继续添加定时事件，得到新事件到期事件
        m_expiration = addTime(now, m_interval);
    }
    else 
    {
        m_expiration = Timestamp();
    }
}