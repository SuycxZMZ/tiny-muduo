#include "tinymuduo.h"

using namespace tinymuduo;

void helper() 
{
    std::cout << "please input like this : ./server ip port" << std::endl;
}

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
        : server_(loop, addr, name)
        , loop_(loop)
    {
        // 注册回调函数
        server_.setConnCallBack(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        
        server_.setMsgCallBack(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置合适的subloop线程数量
        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
    }

private:
    // 连接建立或断开的回调函数
    void onConnection(const TcpConnectionPtr &conn)   
    {
        if (conn->connected())
        {
        }
        else
        {
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        // printf("onMessage!\n");
        std::string msg = buf->retriveAllAsString();
        conn->send(msg);
        // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        helper();
        exit(0);
    }
    
    tinymuduo::initAsyncLogging(argv[0], 1024 * 1024 * 50);
    tinymuduo::AsyncLogStart();

    EventLoop loop;
    InetAddress addr(atoi(argv[2]), argv[1]);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();

    return 0;
}