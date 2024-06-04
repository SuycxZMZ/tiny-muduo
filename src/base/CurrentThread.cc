#include "CurrentThread.h"

namespace tinymuduo
{
namespace CurrentThread {
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 0;
__thread const char* t_threadName = "unknown";

void cacheTid() {
    if (t_cachedTid == 0) {
        // 系统调用获取当前线程id
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}
}  // namespace CurrentThread
} // namespace tinymuduo
