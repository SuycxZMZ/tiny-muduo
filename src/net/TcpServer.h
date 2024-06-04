#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <string>
#include <atomic>

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "CallBacks.h"
#include "Buffer.h"
#include "TimeStamp.h"
#include "AsyncLogging.h"

namespace tinymuduo
{
// 对外暴露的 TcpServer 编程接口
class TcpServer : noncopyable
{
public:
    using ThreadInitCallBack = std::function<void (EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop * loop, 
              const InetAddress & listenAddr,
              const std::string & nameArgs,
              Option option = kNoReusePort);
    ~TcpServer();

    const std::string & ipPort() const { return m_ipPort; }
    const std::string & name() const { return m_name; }
    EventLoop * getLoop() const { return m_loop; }

    void setThreadNum(int numThreads);
    void setThreadInitCallBack(const ThreadInitCallBack & cb)
    { m_threadInitCallBack = std::move(cb); }

    std::shared_ptr<EventLoopThreadPool> threadPool()
    { return m_threadPool; }

    void start();

    void setConnCallBack(const ConnectionCallBack & cb)
    { m_connCallback = cb; }

    void setMsgCallBack(const MsgCallback & cb)
    { m_msgCallBack = cb; }

    void setWriteCompleteCallBack(const WriteCompleteCallBack & cb)
    { m_writeCompleteCallBack = cb; }
private:

    void newConn(int sockfd, const InetAddress & peerAddr);

    void removeConn(const TcpConnectionPtr & conn);
    void removeConnInLoop(const TcpConnectionPtr & conn);

    using ConnMap = std::unordered_map<std::string, TcpConnectionPtr>;
    // main_loop/acceptor_loop
    EventLoop * m_loop;
    const std::string m_ipPort;
    const std::string m_name;
    // 运行在main_loop
    std::unique_ptr<Acceptor> m_acceptor;

    // one loop per thread
    std::shared_ptr<EventLoopThreadPool> m_threadPool;

    ConnectionCallBack m_connCallback;
    MsgCallback m_msgCallBack;
    WriteCompleteCallBack m_writeCompleteCallBack;

    ThreadInitCallBack m_threadInitCallBack;
    std::atomic_int32_t m_started;
    int m_nextConnId;

    ConnMap m_connections;
};
}

#endif