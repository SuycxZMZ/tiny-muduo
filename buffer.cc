#include <errno.h>
#include <sys/uio.h>

#include "buffer.h"

/**
 * poller工作在 LT 模式，只要fd上有数据可读就会一直上报
 * buffer是大小有限制的，但从fd上读数据时，并不知道tcp最终会发送多少数据
*/
ssize_t Buffer::readFd(int fd, int * saveErrno)
{
    // 开辟一个栈上空间 64k
    char extrabuf[65536] = {0};
    // readv, writev 两个函数可以读写多个非连续缓冲区，缓冲区信息存在 iovec 参数中
    struct iovec vec[2];
    // buffer缓冲区剩余空间大小，不一定可以存下读出来的数据
    const size_t writable = writableBytes();

    // buffer 与新开辟的 extrabuf 一起读取，数据顺序填充 vec
    vec[0].iov_base = begin() + m_writeIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable)
    {
        m_writeIndex += n;
    }
    else
    {
        m_writeIndex = m_buffer.size();
        append(extrabuf, n - writable);
    }
    return n;
}