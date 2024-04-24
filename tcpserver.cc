#include <strings.h>
#include <functional>

#include "tcpserver.h"
// #include "logger.h"
#include "utils.h"

TcpServer::TcpServer(EventLoop * loop, 
            const InetAddress & listenAddr,
            const std::string & nameArgs,
            Option option) :
    m_loop(CheckLoopNotNull(loop)),
    m_ipPort(listenAddr.toIpPort()),
    m_name(nameArgs),
    m_acceptor(new Acceptor(loop, listenAddr, option = kReusePort)),
    m_threadPool(new EventLoopThreadPool(loop, m_name)),
    m_connCallback(ConnectionCallBack()),
    m_msgCallBack(MsgCallback()),
    m_nextConnId(1),
    m_started(0)
{
    m_acceptor->setNewConnCallBack(std::bind(&TcpServer::newConn, 
                                             this, 
                                             std::placeholders::_1, 
                                             std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    LOG_INFO("TcpServer::~TcpServer {} destructing", m_name.c_str());
    for (auto & it : m_connections)
    {
        TcpConnectionPtr conn(it.second);
        it.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if (m_started++ == 0)
    {
        m_threadPool->start(m_threadInitCallBack);
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
}

/**
 * 轮询选择一个 subloop
 * 唤醒 subloop
 * 把当前的 connfd 封装成 channel 分发给subloop
 * */
void TcpServer::newConn(int sockfd, const InetAddress & peerAddr)
{
    EventLoop * ioLoop = m_threadPool->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", m_ipPort.c_str(), m_nextConnId);
    ++m_nextConnId;
    std::string connName = m_name + std::string(buf);

    LOG_INFO("TcpServer::newConnection {} - new connection {} from {}", 
             m_name.c_str(), 
             connName.c_str(), 
             peerAddr.toIpPort().c_str());
    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;
    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
    {
        LOG_ERROR("TcpServer::newConnection getsockname");
    }
    InetAddress localAddr(local);

    // localAddr, peerAddr
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    m_connections[connName] = conn;
    conn->setConnectionCallBack(m_connCallback);
    conn->setMsgCallBack(m_msgCallBack);
    // conn->setCloseCallBack(std::bind(&TcpServer::removeConn, this, _1));
    conn->setWriteCompleteCallBack(m_writeCompleteCallBack);
    conn->setCloseCallBack(std::bind(&TcpServer::removeConn, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConn(const TcpConnectionPtr & conn)
{
    m_loop->runInLoop(std::bind(&TcpServer::removeConnInLoop, this, conn));
}

void TcpServer::removeConnInLoop(const TcpConnectionPtr & conn)
{
    LOG_INFO("TcpServer::removeConnInLoop {} - connection {}", m_name.c_str(), conn->name().c_str());
    m_connections.erase(conn->name());
    // 获取conn所在的loop
    EventLoop * ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
