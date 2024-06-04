#ifndef TINYMUDUO_H
#define TINYMUDUO_H

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "CallBacks.h"
#include "Buffer.h"
#include "TimeStamp.h"
#include "TcpServer.h"
#include "Logging.h"
#include "AsyncLogging.h"

namespace tinymuduo
{
std::shared_ptr<tinymuduo::AsyncLogging> g_asyncLog;

// static symlog::AsyncLogging* g_asyncLog = NULL;

void asyncLog(const char* msg, int len) {
    if (tinymuduo::g_asyncLog) {
        tinymuduo::g_asyncLog->append(msg, len);
    }
}

void initAsyncLogging(const char* filename, const off_t RollSize) {
    tinymuduo::g_asyncLog.reset(new tinymuduo::AsyncLogging(filename, RollSize));
    tinymuduo::Logger::setOutput(tinymuduo::asyncLog);
}

void AsyncLogStart() {
    if (tinymuduo::g_asyncLog) {
        tinymuduo::g_asyncLog->start();
    }
}
} // namespace tinymuduo


#endif