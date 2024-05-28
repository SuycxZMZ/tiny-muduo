#ifndef MUDUOTHREAD_H
#define MUDUOTHREAD_H

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <unistd.h>
#include <atomic>


#include "noncopyable.h"

class muduoThread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit muduoThread(ThreadFunc func, const std::string & name = std::string());
    ~muduoThread();

    void start();
    void join();

    bool started() const { return m_started; }
    pid_t tid() const { return m_tid; }
    const std::string & name() const { return m_name; }
    static int numCreate() { return m_numCreate; }
private:
    void setDefaultName();
    bool m_started;
    bool m_joined;
    std::shared_ptr<std::thread> m_thread;
    pid_t m_tid;
    ThreadFunc m_func;
    static std::atomic_int m_numCreate;
    std::string m_name;
};

#endif