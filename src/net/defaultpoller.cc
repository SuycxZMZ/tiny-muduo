#include "poller.h"
#include "epollpoller.h"

#include <stdlib.h>

Poller * Poller::newDefaultPoller(EventLoop * loop)
{
    if (::getenv("TINY_MUDUO_USE_POLL"))
    {
        // poll实例 add code
        return new EpollPoller(loop);
    }
    else
    {
        // epoll实例
        return new EpollPoller(loop);
    }
}