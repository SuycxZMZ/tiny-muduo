### 走一遍用户连接逻辑

回显服务器：
```C++
#include <tinymuduo/tcpserver.h>

#include <functional>
#include <string>

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const std::string &name) : 
               m_loop(loop),
               m_server(m_loop, listenAddr, name)
    {
        LOG_DEBUG("EchoServer::EchoServer()");
        // 注册回调
        m_server.setConnCallBack(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        m_server.setMsgCallBack(std::bind(&EchoServer::onMessage, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3));
        // 设置线程数
        m_server.setThreadNum(3);
    }
    void start()
    {
        m_server.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("client:%s connected", conn->peerAddr().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("client:%s disconnected", conn->peerAddr().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp time)
    {
        std::string msg(buf->retriveAllAsString());
        conn->send(msg);
        conn->shutdown(); // close write --> epoll closeCallBack
    }

    EventLoop *m_loop;
    TcpServer m_server;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000);

    // 建立acceptor，创建 non-blocking listenfd，bind
    EchoServer server(&loop, addr, "EchoServer");

    // listen， 创建loopthread，将listenfd打包为channel向main_loop注册
    server.start();

    loop.loop();

    return 0;
}
```

### 连接建立

![alt text](photos/connect.png)

    1. TcpServer::TcpServer( )创建一个TcpServer对象，即执行代码TcpServer server(&loop, listenAddr); 调用了TcpServer的构造函数，TcpServer构造函数最主要的就是类的内部使用主Event_loop实例化了一个Acceptor对象，并往这个Acceptor对象注册了一个回调函数TcpServer::newConnection()。

    5. Acceptor::Acceptor( )当我们在TcpServer构造函数实例化Acceptor对象时，Acceptor的构造函数中实例化了一个Channel对象，即acceptChannel_，该Channel对象封装了服务器监听套接字文件描述符（尚未注册到main EventLoop的事件监听器上）。接着Acceptor构造函数将Acceptor::handleRead( )方法注册进acceptChannel_中，这也意味着，日后如果事件监听器监听到acceptChannel_发生可读事件，将会调用AcceptorC::handleRead( )函数。

    至此，TcpServer对象创建完毕，用户调用TcpServer::start( )方法，开启TcpServer。来直接看一下TcpServer::start( )方法都干了什么
```C++
void TcpServer::start()
{
    if (m_started++ == 0)
    {
        m_threadPool->start(m_threadInitCallBack);
        // 主要逻辑
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
    }
}

void Acceptor::listen()
{
    m_listenning = true;
    m_acceptSocket.linsten();
    m_acceptChannel.enableReading();
}
```
    其实就是将其实主要就是调用Acceptor::listen( )函数（底层是调用了linux的函数listen( )）监听服务器套接字，以及将acceptChannel_注册到main EventLoop的事件监听器上监听它的可读事件（新用户连接事件）

    接着用户调用loop.loop( )，即调用了EventLoop::loop( )函数，该函数就会循环的获取事件监听器的监听结果，并且根据监听结果调用注册在事件监听器上的Channel对象的事件处理函数。

    6. Acceptor::handleRead( )当程序如果执行到了这个函数里面，说明acceptChannel_发生可读事件，程序处理新客户连接请求。该函数首先调用了Linux的函数accept( )接受新客户连接。接着调用了TcpServer::newConnection( )函数，这个函数是在步骤1中注册给Acceptor并由成员变量newConnectionCallback_保存。

    7. TcpServer::newConnection( )该函数的主要功能就是将建立好的连接进行封装（封装成TcpConnection对象），并使用选择算法公平的选择一个sub EventLoop，并调用TcpConnection::connectEstablished( )将TcpConnection::channel_注册到刚刚选择的sub EventLoop上。


