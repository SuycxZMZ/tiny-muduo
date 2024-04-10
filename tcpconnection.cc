#include <functional>

#include "tcpconnection.h"
#include "eventloop.h"
#include "muduosocket.h"
#include "logger.h"
#include "channel.h"

TcpConnection::TcpConnection(EventLoop * loop,
                const std::string & name,
                int sockfd,
                const InetAddress & localAddr,
                const InetAddress & peerAddr) :
    m_loop(CheckLoopNotNull(loop)),
    m_name(name),
    m_state(kConnecting),
    m_reading(true),
    m_socket(new muduoSocket(sockfd)),
    m_channel(new Channel(loop, sockfd)),
    m_localAddr(localAddr),
    m_peerAddr(peerAddr),
    m_highWaterMark(64*1024*1024)
{
    m_channel->setReadCallBack(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    m_channel->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallBack(std::bind(&TcpConnection::handleError, this));

    LOG_DEBUG("TcpConnection::ctor[%s] at %p fd=%d", m_name.c_str(), this, sockfd);

    m_socket->setKeepAlive(true);
}
TcpConnection::~TcpConnection()
{
    LOG_DEBUG("TcpConnection::dtor[%s] at %p fd=%d", m_name.c_str(), this, m_channel->fd());
}

// 发送数据
void TcpConnection::send(const void * msg, int len)
{
    if (m_state == kConnected)
    {
        if (m_loop->isInLoopThread())
        {
            sendInLoop(msg, len);
        }
        else
        {
            
        }
    }
}

// 关闭连接
void TcpConnection::shutdown()
{

}