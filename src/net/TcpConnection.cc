#include <functional>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "TcpConnection.h"
#include "EventLoop.h"
#include "MuduoSocket.h"
#include "Logging.h"
#include "Channel.h"

namespace tinymuduo
{
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

    LOG_DEBUG << "TcpConnection::ctor[" << m_name << "] at " << this ;

    m_socket->setKeepAlive(true);
}
TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this ;
}

// 发送数据
void TcpConnection::send(const std::string & msg)
{
    if (m_state == kConnected)
    {
        if (m_loop->isInLoopThread())
        {
            sendInLoop(msg.c_str(), msg.size());
        }
        else
        {
            m_loop->runInLoop(std::bind(&TcpConnection::sendInLoop,
                              this, msg.c_str(), msg.size()));
        }
    }
}

/**
 * 发送数据的实现
 * 应用只管写，内核发送的慢，设置有高水位线
*/
void TcpConnection::sendInLoop(const void * msg, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (kDisconnected == m_state)
    {
        return;
    }

    // if no thing in output queue, try writing directly
    if (!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0)
    {
        nwrote = ::write(m_channel->fd(), msg, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            // 发完了, 用不上缓冲区，也就不用再往channel上注册epollout事件
            if (remaining == 0 && m_writeCompleteCallBack)
            {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallBack, shared_from_this()));
            }
        }
        else // nwrote < 0 出错
        {
            nwrote = 0;
            // EWOULDBLOCK 由于非阻塞，没有返回
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR << "TcpConnection::sendInLoop";
                // 对端 SIGPIPE 或者 ECONNRESET
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }

    /**
     * 当前这次write系统调用并没有把数据全部发送出去
     * 剩余的数据保存在缓冲区，然后给channel注册 epollout 事件
     * poller发现tcp的发送缓冲区有空间时就会通知相应的channel调用 writeCallBack
     * 在tcpconnection中给channel注册的writeCallBack就是handleWrite
     * */ 
    if (!faultError && remaining > 0)
    {
        size_t oldLen = m_outputBuffer.readableBytes();
        if (oldLen + remaining >= m_highWaterMark
            && oldLen < m_highWaterMark
            && m_highWaterMarkCallBack)
        {
            m_loop->queueInLoop(std::bind(m_highWaterMarkCallBack,
                                          shared_from_this(),
                                          oldLen + remaining)); 
        }
        m_outputBuffer.append(static_cast<const char*>(msg) + nwrote, remaining);
        if (!m_channel->isWriting())
        {
            m_channel->enableWriting();
        }
    }
}

//  
void TcpConnection::shutdownInLoop()
{
    if (!m_channel->isWriting())
    {
        m_socket->shutdownWrite();
    }
}

// 关闭连接
void TcpConnection::shutdown()
{
    if (m_state == kConnected)
    {
        setState(kDisconnecting);
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

// called when TcpServer accepts a new connection
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallBack(shared_from_this());
}

// called when TcpServer has removed me from its map
void TcpConnection::connectDestroyed()
{
    if (m_state == kConnected)
    {
        setState(kDisconnected);
        // 把channel感兴趣的事件从poller中移除
        m_channel->disableAll();
        m_connectionCallBack(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &saveErrno);
    if (n > 0)
    {
        m_msgCallBack(shared_from_this(), &m_inputBuffer, receiveTime);
    }
    else if (n == 0)    // client 断开
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_ERROR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (m_channel->isWriting())
    {
        int saveErrno = 0;
        ssize_t n = m_outputBuffer.writeFd(m_channel->fd(), &saveErrno);
        if (n > 0)
        {
            m_outputBuffer.retrive(n);
            // 已经发送完了
            if (m_outputBuffer.readableBytes() == 0)
            {
                m_channel->disableWriting();
                // 执行用户注册的回调
                if (m_writeCompleteCallBack)
                {
                    // 每个 connection channel 单独包含于一个loop, 也可以直接用 runInLoop
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallBack, shared_from_this()));
                }
                if (m_state == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR << "TcpConnection::handleWrite";
        }
    }
    else
    {
        LOG_ERROR << "TcpConnection::handleWrite state=" << m_state;
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO << "TcpConnection::handleClose state=" << m_state;
    setState(kDisconnected);
    m_channel->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    m_connectionCallBack(guardThis);
    // must be the last line
    m_closeCallBack(guardThis);
}

void TcpConnection::handleError()
{
    int err = 0;
    int optval = 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(m_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR << "TcpConnection::handleError [" << m_name << "]" ;
}
} // namespace tinymuduo



