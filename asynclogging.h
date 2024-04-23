#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "logstream.h"
#include "noncopyable.h"

namespace Log
{
class AsyncLogging : noncopyable
{

};
} // namespace Log



#endif // ASYNCLOGGING_H