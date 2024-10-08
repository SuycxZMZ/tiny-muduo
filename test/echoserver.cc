#include "Logging.h"
#include "TcpServer.h"
#include <functional>
#include <string>

using namespace tinymuduo;

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const std::string &name) : 
               m_loop(loop),
               m_server(m_loop, listenAddr, name)
    {
        LOG_INFO << "EchoServer::EchoServer()" ;
        // 注册回调
        m_server.setConnCallBack(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        m_server.setMsgCallBack(std::bind(&EchoServer::onMessage, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3));
        // 设置线程数
        m_server.setThreadNum(2);
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
        }
        else
        {
            conn->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp time)
    {
        std::string msg(buf->retriveAllAsString());
        conn->send(msg);
        // conn->shutdown(); // close write --> epoll closeCallBack
    }

    EventLoop *m_loop;
    TcpServer m_server;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000);

    Logger::setLogLevel(Logger::LogLevel::WARN);

    // 建立acceptor，创建 non-blocking listenfd，bind
    EchoServer server(&loop, addr, "EchoServer");

    // listen， 创建loopthread，将listenfd打包为channel向main_loop注册
    server.start();

    loop.loop();

    return 0;
}
