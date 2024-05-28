#ifndef CHANNEL_H
#define CHANNEL_H

#include "noncopyable.h"
#include "timestamp.h"

#include <functional>
#include <memory>

class EventLoop;
// class Timestamp;

// 封装了 sockfd 和 感兴趣的 event ，还包含了 poller返回的具体事件
class Channel : noncopyable
{
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(Timestamp)>;
    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd 的到 poller 的通知之后处理事件，进行相应的回调
    void handelEvent(Timestamp receiveTime);

    // 设置具体的回调函数对象
    void setReadCallBack(ReadEventCallBack cb) { m_readCallBack = std::move(cb); }
    void setWriteCallBack(EventCallBack cb) { m_writeCallBack = std::move(cb); }
    void setCloseCallBack(EventCallBack cb) { m_closeCallBack = std::move(cb); }
    void setErrorCallBack(EventCallBack cb) { m_errorCallBack = std::move(cb); }

    // 防止手动 remove 掉 channel 之后，还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() { return m_fd; }
    int events() { return m_events; }
    // poller持有channel指针来设置
    void set_revents(int revt) { m_revents = revt; }

    void enableReading() { m_events |= kReadEvent; update(); }
    void disableReading() { m_events &= ~kReadEvent; update(); }
    void enableWriting() { m_events |= kWriteEvent; update(); }
    void disableWriting() { m_events |= kWriteEvent; update(); }
    void disableAll() { m_events = kNoneEvent; update(); }
    bool isWriting() const { return m_events & kWriteEvent; }
    bool isReading() const { return m_events & kReadEvent; }
    bool isNoneEvent() const { return m_events == kNoneEvent; }

    // poller 使用
    int index() { return m_index; }
    void set_index(int idx) { m_index = idx; }

    // 每个 channel 只能被一个 loop 包含
    EventLoop * ownerLoop() { return m_loop; }

    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp recvTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *m_loop; // 事件循环
    int m_fd;          // poller监听的对象
    int m_events;      // fd 上感兴趣的事件
    int m_revents;     // poller返回的具体发生的事件
    int m_index;

    std::weak_ptr<void> m_tie;
    bool m_tied;

    // channel 可以获知 fd 里面具体发生的事件 revents ， 它负责具体事件的回调操作
    ReadEventCallBack m_readCallBack;
    EventCallBack m_writeCallBack;
    EventCallBack m_closeCallBack;
    EventCallBack m_errorCallBack;
};

#endif