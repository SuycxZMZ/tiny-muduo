#include <errno.h>
#include <unistd.h>
#include <strings.h>

#include "epollpoller.h"
#include "logger.h"

// 与 channel 中的 index 做判断，表示其在poller中的状态
// channel 未添加到poller中
const int kNew = -1;
// channel 已添加到poller中
const int kAdded = 1;
// channel 从poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop * loop) : 
    Poller(loop),
    m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    m_events(kInitEventListSize)
{
    if (m_epollfd < 0)
    {
        LOG_FATAL("create epollfd error : %d", errno);
    }
}

EpollPoller::~EpollPoller()
{
    ::close(m_epollfd);
}

/**
 * epoll_wait
 * 
*/
Timestamp EpollPoller::poll(int timeoutMs, ChannelList * activeChannels)
{
    LOG_DEBUG("func = %s, total fd cnt = %d", __FUNCTION__, m_channels.size());
    int numEvents = ::epoll_wait(m_epollfd, 
                                 &*m_events.begin(),
                                 static_cast<int>(m_events.size()),
                                 timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG_DEBUG("%d events happened", numEvents);
        fillActivateChannels(numEvents, activeChannels);
    }
    else
    {

    }
}

void EpollPoller::updateChannel(Channel * channel)
{
    const int index = channel->index();
    LOG_INFO("func=%s fd = %d events = %d index = %d",__FUNCTION__, channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            m_channels[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // index == kAdded
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

/**
 * 从poller的map中删除对应的channel
 * 从epoll中删除对应的events
 * event_loop的channel_list不删除
 */
void EpollPoller::removeChannel(Channel * channel)
{
    int fd = channel->fd();
    m_channels.erase(fd);
    int index = channel->index();
    LOG_INFO("func = %d, fd = %d, index = %d", __FUNCTION__, fd, index);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃连接
void EpollPoller::fillActivateChannels(int numEvents,
                            ChannelList * actChannels) const
{

}
// 更新 channel
void EpollPoller::update(int operation, Channel * channel)
{
    epoll_event event;
    int fd = channel->fd();
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    event.data.fd = fd;
    if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("delete error, epoll_ctl_op = %d, fd = % d", operation, fd);
        }
        else
        {
            LOG_FATAL("add or modify error, epoll_ctl_op = %d, fd = % d", operation, fd);
        }
    }
}