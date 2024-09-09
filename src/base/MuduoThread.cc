#include <bits/types/struct_sched_param.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "MuduoThread.h"
#include "CurrentThread.h"
#include "Logging.h"

namespace tinymuduo
{
std::atomic_int muduoThread::m_numCreate(0);

muduoThread::muduoThread(ThreadFunc func, const std::string & n) :
    m_started(false),
    m_joined(false),
    m_tid(0),
    m_func(std::move(func)),
    m_name(n)
{
    setDefaultName();
}

muduoThread::~muduoThread()
{
    if (m_started && !m_joined)
    {
        m_thread->detach();
    }
}

// 一个新的muduoThread对象记录了一个新线程的详细信息
void muduoThread::start()
{
    m_started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    m_thread = std::shared_ptr<std::thread> (new std::thread([&](){
        // 获取线程id
        m_tid = CurrentThread::tid();
        sem_post(&sem);
        // 开启一个新线程，执行线程函数
        m_func();
    }));

    // 等待上面新线程的tid
    sem_wait(&sem);
}

void muduoThread::join()
{
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    if (m_thread->joinable())
    {
        m_thread->join();
    }
}

void muduoThread::setDefaultName()
{
    int num = ++m_numCreate;
    if (m_name.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        m_name = std::string(buf);
    }
}

void muduoThread::setThreadPriority() {
    if (!m_thread) {
        LOG_ERROR << "Thread has not been created";
        return;
    }

    // 获取原生handle句柄
    pthread_t native_handle = m_thread->native_handle();

    // // 设置调度策略为SCHED_BATCH（批处理策略）
    int policy = SCHED_BATCH;
    struct sched_param param = {}; // 使用默认的批处理优先级

    // 使用pthread_setschedparam设置线程优先级
    if (pthread_setschedparam(native_handle, policy, &param) != 0) {
        LOG_ERROR << "Failed to set thread priority";
    } else {
        LOG_INFO << "Thread priority set to " << "SCHED_BATCH";
    }
}

} // namespace tinymuduo
