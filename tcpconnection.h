#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <memory>
#include <string>
#include <atomic>

#include "noncopyable.h"
#include "inetaddress.h"
#include "callbacks.h"
#include "buffer.h"
#include "timestamp.h"

class Channel;
class EventLoop;
class muduoSocket;

/**
 * 封装 socket channel client_callback，已经建立连接的链路
 * TcpServer -> Acceptor -> 新用户连接，通过accept函数得到一个connfd
 *  -> TcpConnection 设置回调 -> Channel -> Poller -> Channel的回调操作
 * */ 
class TcpConnection : noncopyable , public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop * loop,
                  const std::string & name,
                  int sockfd,
                  const InetAddress & localAdd,
                  const InetAddress & peerAddr);
    ~TcpConnection();

    EventLoop * getLoop() const { return m_loop; }
    const std::string name() const { return m_name; }
    const InetAddress & localAddr() const { return m_localAddr; }
    const InetAddress & peerAddr() const { return m_peerAddr; }
    bool connected() const { return m_state == kConnected; }
    bool disconnected() const { return m_state == kDisconnected; }

    // 发送数据
    void send(const std::string & msg);
    // void sendInLoop(const std::string & msg);
    // 关闭连接
    void shutdown();
    // void shutdownInLoop();

    void setConnectionCallBack(const ConnectionCallBack & cb)
    { m_connectionCallBack = cb; }
    void setMsgCallBack(const MsgCallback & cb)
    { m_msgCallBack = cb; }
    void setWriteCompleteCallBack(const WriteCompleteCallBack & cb)
    { m_writeCompleteCallBack = cb; }
    void setHighWaterMarkCallBack(const HighWaterMarkCallback & cb, size_t highWaterMark)
    { m_highWaterMarkCallBack = cb; m_highWaterMark = highWaterMark; }
    void setCloseCallBack(const CloseCallBack & cb)
    { m_closeCallBack = cb; }

    Buffer * inputBuffer() {return &m_inputBuffer; }
    Buffer * outputBuffer() { return &m_outputBuffer; }

    // called when TcpServer accepts a new connection
    void connectEstablished(); // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed(); // should be called only once
private:
    // 连接状态
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void setState(StateE s) { m_state = s; }

    void sendInLoop(const void * msg, size_t len);
    void shutdownInLoop();

    // sub_loop TcpConnection都是在sub_loop中管理
    EventLoop * m_loop;
    const std::string m_name;
    std::atomic_int m_state;
    bool m_reading;

    std::unique_ptr<muduoSocket> m_socket;
    std::unique_ptr<Channel> m_channel;

    // 当前主机地址
    const InetAddress m_localAddr;
    // 对端地址
    const InetAddress m_peerAddr;

    ConnectionCallBack m_connectionCallBack;
    MsgCallback m_msgCallBack;
    WriteCompleteCallBack m_writeCompleteCallBack;
    CloseCallBack m_closeCallBack;
    HighWaterMarkCallback m_highWaterMarkCallBack;

    size_t m_highWaterMark;

    // 接收缓冲区
    Buffer m_inputBuffer;
    // 发送缓冲区
    Buffer m_outputBuffer;
};

#endif