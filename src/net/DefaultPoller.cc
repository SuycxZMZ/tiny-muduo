#include "Poller.h"
#include "EpollPoller.h"

#include <stdlib.h>

namespace tinymuduo
{
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
} // namespace tinymuduo
