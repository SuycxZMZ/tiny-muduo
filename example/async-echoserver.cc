#include <tinymuduo/tcpserver.h>

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
        server_.setThreadNum(3);
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

int kRollSize = 500*1000*1000;

// 异步日志
std::unique_ptr<AsyncLogging> g_asyncLog;

void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void setLogging(const char* argv0)
{
    Logger::setOutput(asyncOutput);
    // Logger::setLogLevel(Logger::LogLevel::WARN);
    char name[256];
    strncpy(name, argv0, 256);
    g_asyncLog.reset(new AsyncLogging(::basename(name), kRollSize));
    g_asyncLog->start();
}


int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        helper();
        exit(0);
    }

    setLogging(argv[0]);
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress addr(atoi(argv[2]), argv[1]);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();

    return 0;
}