#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>

#include "noncopyable.h"
#include "MuduoSocket.h"
#include "Channel.h"

namespace tinymuduo
{
class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnCallBack = std::function<void (int sockfd, const InetAddress&)>;

    Acceptor(EventLoop * loop, const InetAddress & listenaddr, bool reuseport);
    ~Acceptor();

    void setNewConnCallBack(const NewConnCallBack & cb) { m_newConnCallBack = cb; }

    void listen();
    bool listening() const { return m_listenning; }

private:
    // 有新用户连接执行的回调
    void handleRead();

    // main_loop
    EventLoop * m_loop;

    // acceptfd 绑定到 listen_addr，设置非阻塞
    muduoSocket m_acceptSocket;
    Channel m_acceptChannel;

    // 来了一条新连接的回调
    NewConnCallBack m_newConnCallBack;
    bool m_listenning;
};
} // namespace tinymuduo



#endif