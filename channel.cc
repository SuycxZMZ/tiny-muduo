#include <sys/epoll.h>

#include "channel.h"
#include "eventloop.h"
// #include "logger.h"
#include "utils.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) :
        m_loop(loop),
        m_fd(fd),
        m_events(0),
        m_revents(0),
        m_index(-1),
        m_tied(false)
{
}
Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void> & obj)
{
    m_tie = obj;
    m_tied = true;
}

/*
当改变 fd 的 events事件后，update负责通过poller更改fd对应的事件 epoll_ctl
Eventloop ==> channel_list + poller
*/ 
void Channel::update()
{
    m_loop->updateChannel(this);
}

void Channel::remove()
{
    m_loop->removeChannel(this);
}

void Channel::handelEvent(Timestamp receiveTime)
{
    if (m_tied)
    {
        std::shared_ptr<void> guard = m_tie.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp recvTime)
{
    LOG_INFO("channel handle revent : {}", m_revents);
    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN))
    {
        if (m_closeCallBack) m_closeCallBack();
    }

    if (m_revents & EPOLLERR)
    {
        if (m_errorCallBack) m_errorCallBack();
    }

    if (m_revents & (EPOLLIN | EPOLLPRI))
    {
        if (m_readCallBack) m_readCallBack(recvTime);
    }

    if (m_revents & EPOLLOUT)
    {
        if (m_writeCallBack) m_writeCallBack();
    }
}