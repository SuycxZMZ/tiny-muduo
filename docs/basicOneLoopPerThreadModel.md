# One Loop Per Thread Model

One Loop Per Thread的含义就是，一个EventLoop和一个线程唯一绑定，和这个EventLoop有关的，被这个EventLoop管辖的一切操作都必须在这个EventLoop绑定线程中执行，比如在MainEventLoop中，负责新连接建立的操作都要在MainEventLoop线程中运行。
已建立的连接分发到某个SubEventLoop上，这个已建立连接的任何操作，比如接收数据发送数据，连接断开等事件处理都必须在这个SubEventLoop线程上运行，不准跑到别的SubEventLoop线程上运行。

## 1. evevtfd() 函数

```c
#include <sys/eventfd.h>
int eventfd(unsigned int initval, int flags);
```
调用函数eventfd()会创建一个eventfd对象，或者也可以理解打开一个eventfd类型的文件，类似普通文件的open操作。eventfd的在内核空间维护一个无符号64位整型计数器， 初始化为initval的值。

## 如何保证一个EventLoop对象和一个线程唯一绑定

```C++
// eventloop.cc
// 防止一个线程创建多个 event_loop
__thread EventLoop * t_loopInThisThread = nullptr;

EventLoop::EventLoop() :
    ...
    m_wakeupFd(createEventfd()),
    m_wakeupChannel(new Channel(this, m_wakeupFd)),
    m_currentActiveChannel(nullptr)
{
    //如果当前线程已经绑定了某个EventLoop对象了，那么该线程就无法创建新的EventLoop对象了
    if (t_loopInThisThread)
    {
        LOG_FATAL(" ");
    }
    else
    {
        t_loopInThisThread = this;
    }
    m_wakeupChannel->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    // 每个loop都监听 wakeupfd 上的读事件，main_loop 负责在每个 sub_loop 的wakeupfd上写，唤醒sub_loop
    m_wakeupChannel->enableReading();
}
```

在EventLoop对象的构造函数中，如果当前线程没有绑定EventLoop对象，那么t_loopInThisThread为nullptr，然后就让该指针变量指向EventLoop对象的地址。如果t_loopInThisThread不为nullptr，说明当前线程已经绑定了一个EventLoop对象了，这时候EventLoop对象构造失败！

## Muduo库如何实现每个Loop线程只运行隶属于该EventLoop对象的操作？

比如在MainEventLoop中，负责新连接建立的操作都要在MainEventLoop线程中运行。已建立的连接分发到某个SubEventLoop上之后，这个已建立连接的任何操作，比如接收数据发送数据，连接断开等事件处理都必须在这个SubEventLoop线程上运行，不准跑到别的SubEventLoop线程上运行。
EventLoop构造函数的初始化列表中，如下所示：
```C++
// 创建 wakeupfd，用来notify唤醒 subloop
int createEventfd()
{  
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL(" ");
    }
    return evtfd;
}
EventLoop::EventLoop() 
    : m_wakeupFd(createEventfd()), //生成一个eventfd，每个EventLoop对象，都会有自己的eventfd
	...
{...}
```

在 EventLoop 的初始化列表中：

- CreateEventfd()返回一个eventfd文件描述符，并且该文件描述符设置为非阻塞和子进程不拷贝模式。该eventfd文件描述符赋给了EventLoop对象的成员变量 wakeupFd_。

- 随即将wakeupFd_用Channel封装起来，得到 wakeupChannel_ 。
  
我们来描绘一个情景，我们知道每个EventLoop线程主要就是在执行其EventLoop对象的loop函数（该函数就是一个while循环，循环的获取事件监听器的结果以及调用每一个发生事件的Channel的事件处理函数）。此时 sub_loop 上注册的Tcp连接都没有任何动静，整个 sub_loop 线程就阻塞在 epoll_wait() 上。

此时 main_loop 接受了一个新连接请求，并把这个新连接封装成一个TcpConnection对象，并且希望在 sub_loop 线程中执行 TcpConnection::connectEstablished() 函数，因为该函数的目的是将TcpConnection注册到 sub_loop 的事件监听器上，并且调用用户自定义的连接建立后的处理函数。当该TcpConnection对象注册到 sub_loop 之后，这个TcpConnection对象的任何操作（包括调用用户自定义的连接建立后的处理函数。）都必须要在这个SubEventLoop线程中运行，所以TcpConnection::connectEstablished()函数必须要在SubEventLoop线程中运行。

那么我们怎么在 main_loop 线程中通知 sub_loop 线程起来执行TcpConnection::connectEstablished()函数呢？这里就要好好研究一下EventLoop::runInLoop()函数了。

```C++
// 在当前loop线程中执行cb
// 该函数保证了cb这个函数对象一定是在其EventLoop线程中被调用。
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else // 在非当前loop的线程中执行cb, 要唤醒loop所在线程，执行cb
    {
        queueInLoop(std::move(cb));
    }
}

// 把cb放入队列，在其他线程执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingFunctors.emplace_back(std::move(cb)); 
    }
    // m_callingPendingFunctors = true 情况下，正在执行回调。wakeup一次，保证再次poll的时候不阻塞
    if (!isInLoopThread() || m_callingPendingFunctors)
    {
        wakeUp();
    }
}

void EventLoop::wakeUp()
{
    uint64_t one = 1;
    ssize_t n = ::write(m_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("");
    }
}

void EventLoop::loop()
{
    m_looping = true;
    m_quit = false;
    LOG_INFO("EventLoop %p start looping", this);

    while (!m_quit)
    {
        m_channelList.clear();
        // main_loop 监听的是 acceptorfd, sub_loop监听的是clientfd
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_channelList);
        for (Channel * channel : m_channelList)
        {
            m_currentActiveChannel = channel;
            m_currentActiveChannel->handelEvent(m_pollReturnTime);
        }
        m_currentActiveChannel = nullptr;
        /**
         * 执行当前Eventloop需要处理的回调操作
         * main_loop --> 只做 accept用，返回 fd --> 打包为channel --> 给sub_loop
         * main_loop 事先注册一个回调(sub_loop使用) sub_loop被唤醒之后执行
        */
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping", this);
    m_looping = false;
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }
    for (const Functor & functor : functors)
    {
        functor();
    }
    m_callingPendingFunctors = false;
}
```

- 首先EventLoop::runInLoop函数接受一个可调用的函数对象Functor cb，如果当前cpu正在运行的线程就是该EventLoop对象绑定的线程，那么就直接执行cb函数。否则就把cb传给queueInLoop()函数。

- 在queueInLoop()函数中主要就是把cb这个可调用对象保存在EventLoop对象的 pendingFunctors_ 这个数组中，我们希望这个cb能在某个EventLoop对象所绑定的线程上运行，但是由于当前cpu执行的线程不是我们期待的这个EventLoop线程，我们只能把这个可调用对象先存在这个EventLoop对象的数组成员 pendingFunctors_ 中。

- 我们再把目光转移到上面代码中的EventLoop::loop()函数中。我们知道EventLoop::loop()肯定是运行在其所绑定的EventLoop线程内，在该函数内会调用**doPendingFunctors()**函数，这个函数就是把自己这个EventLoop对象中的pendingFunctors_数组中保存的可调用对象拿出来执行。**pendingFunctors_中保存的是其他线程希望你这个EventLoop线程执行的函数**。

- 还有一个问题，假如EventLoop A线程阻塞在EventLoop::loop()中的epoll_wait()调用上（EventLoop A上监听的文件描述符没有任何事件发生），这时候EventLoop线程要求EventLoop A赶紧执行某个函数，那其他线程要怎么唤醒这个阻塞住的EventLoopA线程呢？这时候我们就要把目光聚焦在上面的wakeup()函数了。

- wakeup()函数就是向我们想唤醒的线程所绑定的EventLoop对象持有的wakeupFd_随便写入一个8字节数据，因为wakeupFd_已经注册到了这个EventLoop中的事件监听器上，这时候事件监听器监听到有文件描述符的事件发生，epoll_wait()阻塞结束而返回。这就相当于起到了唤醒线程的作用。你这个EventLoop对象既然阻塞在事件监听上，那我就通过wakeup()函数给你这个EventLoop对象一个事件，让你结束监听阻塞。





