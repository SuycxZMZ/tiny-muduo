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
void do_asyncLog(const char* msg, int len);

void initAsyncLogging(const char* filename, const off_t RollSize);

void AsyncLogStart();
} // namespace tinymuduo


#endif