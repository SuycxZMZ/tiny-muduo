#include "HttpServer.h"
#include "Logging.h"
#include <memory>

namespace tinymuduo
{
/**
 * 默认的http回调函数
 * 设置响应状态码，响应信息并关闭连接
 */
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop,
                      const InetAddress &listenAddr,
                      const std::string &name,
                      TcpServer::Option option)
  : server_(loop, listenAddr, name, option),
    httpCallback_(defaultHttpCallback)
{
    server_.setConnCallBack(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMsgCallBack(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server_.setThreadNum(4);
}

void HttpServer::start()
{
    LOG_INFO << "HttpServer[" << server_.name().c_str() << "] starts listening on " << server_.ipPort().c_str();
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << "new Connection arrived";
    }
    else 
    {
        LOG_INFO << "Connection closed";
    }
}

// 有消息到来时的业务处理
void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
    // LOG_INFO << "HttpServer::onMessage";
    std::unique_ptr<HttpContext> context(new HttpContext);

#if 0
    // 打印请求报文
    std::string request = buf->GetBufferAllAsString();
    std::cout << request << std::endl;
#endif

    // 进行 状态机解析
    // 错误则发送 BAD REQUEST 半关闭
    if (!context->parseRequest(buf, receiveTime))
    {
        LOG_ERROR << "parseRequest failed!";
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    // 如果成功解析
    if (context->gotAll())
    {
        // LOG_INFO << "parseRequest success!";
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string& connection = req.getHeader("Connection");

    // 判断长连接还是短连接
    bool close = connection == "close" ||
        (req.version() == HttpRequest::kHttp10 && connection != "Keep-Alive");

    // TODO:强制设的短链接
    close = true;

    // 响应信息
    HttpResponse response(close);

    // httpCallback_ 由用户传入，怎么写响应体由用户决定
    // 此处初始化了一些response的信息，比如响应码，回复OK
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);

    // TODO:需要重载 TcpConnection::send 使其可以接收一个缓冲区
    // 长连接的情况
    // 这只是一个例子，解析http也不用C++来写
    conn->send(buf.retriveAllAsString());
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}
} // namespace tinymuduo
