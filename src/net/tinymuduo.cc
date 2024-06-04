#include "tinymuduo.h"

namespace tinymuduo 
{
void do_asyncLog(const char* msg, int len) {
    if (tinymuduo::g_asyncLog) {
        tinymuduo::g_asyncLog->append(msg, len);
    }
}

void initAsyncLogging(const char* filename, const off_t RollSize) {
    tinymuduo::g_asyncLog.reset(new tinymuduo::AsyncLogging(filename, RollSize));
    tinymuduo::Logger::setOutput(tinymuduo::do_asyncLog);
}

void AsyncLogStart() {
    if (tinymuduo::g_asyncLog) {
        tinymuduo::g_asyncLog->start();
    }
}
}