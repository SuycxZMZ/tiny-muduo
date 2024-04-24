#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>

#include "acceptor.h"
// #include "logger.h"
#include "utils.h"
#include "inetaddress.h"

static int createNonBlockingOrDie()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG_FATAL("{}:create a sockfd:{} error", __FUNCTION__, sockfd);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop * loop, const InetAddress & listenaddr, bool reuseport) :
    m_loop(loop),
    m_acceptSocket(createNonBlockingOrDie()),   // 创建
    m_acceptChannel(m_loop, m_acceptSocket.fd()),
    m_listenning(false)
{
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.setReusePort(reuseport);
    m_acceptSocket.bindAddress(listenaddr);     // 绑定

    // acceptfd --> acceptchannel 上注册的回调，把接收到的 clientfd 打包发送给sub_loop
    m_acceptChannel.setReadCallBack(std::bind(&Acceptor::handleRead, this));
    LOG_INFO("acceptorfd = {}", m_acceptSocket.fd());
}

Acceptor::~Acceptor()
{
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
}

void Acceptor::listen()
{
    m_listenning = true;
    m_acceptSocket.linsten();
    m_acceptChannel.enableReading();
}

// 新用户连接触发 m_acceptChannel 有事件发生
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = m_acceptSocket.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (m_newConnCallBack)
        {
            // 轮询 sub_loop ...
            m_newConnCallBack(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("{}:accept error:{}", __FUNCTION__, errno);
    }
}