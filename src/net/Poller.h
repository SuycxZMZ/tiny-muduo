#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "TimeStamp.h"
#include "EventLoop.h"

namespace tinymuduo
{
class Channel;
class EventLoop;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop * loop);
    virtual ~Poller();

    // Must be called in the loop thread.
    // 给所有 IO 复用保留统一的接口去重写 --> 相当于启动 epoll_wait
    virtual Timestamp poll(int timeoutMs, ChannelList * activeChannels) = 0;

    virtual void updateChannel(Channel * channel) = 0;
    virtual void removeChannel(Channel * channel) = 0;
    virtual bool hasChannel(Channel * channel) const;

    // EventLoop 可以通过该接口获得默认的IO复用具体实现
    static Poller * newDefaultPoller(EventLoop * loop);
protected:
    // fd : channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap m_channels;
private:
    EventLoop * m_ownerloop;
};
}


#endif