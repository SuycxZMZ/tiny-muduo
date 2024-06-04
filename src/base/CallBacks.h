#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <memory>
#include <functional>
#include "TimeStamp.h"

namespace tinymuduo
{
class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallBack = std::function<void()>;
using ConnectionCallBack = std::function<void(const TcpConnectionPtr &)>;
using CloseCallBack = std::function<void(const TcpConnectionPtr &)> ;
using WriteCompleteCallBack = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

using MsgCallback = std::function<void (const TcpConnectionPtr &,
                                        Buffer *,
                                        Timestamp)>;
} // namespace tinymuduo



#endif