#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include <vector>
#include <sys/epoll.h>

#include "poller.h"
#include "channel.h"

/**
* epoll : epoll_create --> epoll_ctl --> epoll_wait
*/
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop * loop);
    ~EpollPoller() override;

    // 重写 poller 的抽象方法
    Timestamp poll(int timeoutMs, ChannelList * activeChannels) override;
    void updateChannel(Channel * channel) override;
    void removeChannel(Channel * channel) override;
private:
    static const int kInitEventListSize = 16;

    // 填写活跃连接
    void fillActivateChannels(int numEvents,
                              ChannelList * actChannels) const;
    // 更新 channel
    void update(int operation, Channel * channel);

    using EventList = std::vector<epoll_event>;
    int m_epollfd;
    EventList m_events;
};



#endif