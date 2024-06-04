#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <cstdio>
#include <cstring>

namespace tinymuduo
{
namespace CurrentThread {

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
void cacheTid();

inline int tid() {
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidString() {
    if (0 == t_cachedTid) {
        int retid = tid();
        snprintf(t_tidString, sizeof(t_tidString), "%d", retid);
        t_tidStringLength = strlen(t_tidString);
    }
    return t_tidString;
}

inline int tidStringLength() {
    if (0 == t_tidStringLength) {
        tidString();
    }
    return t_tidStringLength;
}

inline const char* name() { return t_threadName; }
}  // namespace CurrentThread

} // namespace tinymuduo


#endif