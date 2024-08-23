#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <netinet/in.h>

void on_read(struct bufferevent *bev, void *ctx)
{
    // 读取请求内容（这里我们忽略内容，只简单回应）
    struct evbuffer *input = bufferevent_get_input(bev);
    evbuffer_drain(input, evbuffer_get_length(input));

    // 准备响应
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 13\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, world!";

    // 发送响应
    bufferevent_write(bev, response, strlen(response));
}

void on_event(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR)
    {
        std::cerr << "Error on the connection!" << std::endl;
    }
    bufferevent_free(bev);
}

void on_accept(evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, on_read, nullptr, on_event, nullptr);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void run_event_loop(short port)
{
    struct event_base *base = event_base_new();
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    struct evconnlistener *listener = evconnlistener_new_bind(base, on_accept, (void *)base,
                                                              LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                                              (struct sockaddr *)&sin, sizeof(sin));

    if (!listener)
    {
        std::cerr << "Could not create a listener!" << std::endl;
        return;
    }

    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: http_server <port>" << std::endl;
        return 1;
    }

    short port = std::atoi(argv[1]);

    // 创建6个线程运行事件循环
    std::vector<std::thread> threads;
    for (int i = 0; i < 1; ++i)
    {
        threads.emplace_back([port]()
                             { run_event_loop(port); });
    }

    // 等待所有线程完成
    for (auto &thread : threads)
    {
        thread.join();
    }

    return 0;
}
