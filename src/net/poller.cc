#include "poller.h"
#include "channel.h"

Poller::Poller(EventLoop * loop) : m_ownerloop(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel * channel) const
{
    ChannelMap::const_iterator it = m_channels.find(channel->fd());
    return it != m_channels.end() && it->second == channel;
}

