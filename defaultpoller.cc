#include "poller.h"
#include <stdlib.h>

Poller * Poller::newDefaultPoller(EventLoop * loop)
{
    if (::getenv("TINY_MUDUO_USE_POLL"))
    {
        // poll实例
        return nullptr;
    }
    else
    {
        // epoll实例
        return nullptr;
    }
}