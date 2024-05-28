#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <algorithm>

#include "noncopyable.h"

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer : noncopyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialsize = kInitialSize) :
        m_buffer(kCheapPrepend + initialsize),
        m_readIndex(kCheapPrepend),
        m_writeIndex(kCheapPrepend) {}
    
    // 缓冲区可读空间  --> 以下三个空间相加 = m_buffer.size()
    size_t readableBytes() const { return m_writeIndex - m_readIndex; }

    // 缓冲区后面的空间
    size_t writableBytes() const { return m_buffer.size() - m_writeIndex; }

    // 缓冲区前面的空间
    size_t prependableBytes() const {return m_readIndex; }

    // 返回缓冲区中可读数据的起始地址
    const char * peek() const { return begin() + m_readIndex; }

    void retrive(size_t len)
    {
        if (len < readableBytes())
        {
            m_readIndex += len;
        }
        else
        {
            retriveAll();
        }
    }

    void retriveAll()
    {
        m_readIndex = kCheapPrepend;
        m_writeIndex = kCheapPrepend;
    }

    std::string retriveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrive(len);
        return result;
    }

    // 把 onMsg 上报的数据流从缓冲区读出，按string返回
    std::string retriveAllAsString()
    {
        return retriveAsString(readableBytes());
    }

    // 保证 len 的数据可写
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            // 扩容
            makeSpace(len);
        }
    }

    char * beginWrite() { return begin() + m_writeIndex; }
    const char * beginWrite() const { return begin() + m_writeIndex; }

    // data 写入 Writable 缓冲区
    void append(const char * data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        m_writeIndex += len;
    }

    // 从 fd 读数据到缓冲区
    ssize_t readFd(int fd, int * saveErrno);

    // 通过 fd 写缓冲区 发送数据, 把缓冲区数据写入fd
    ssize_t writeFd(int fd, int * saveErrno);
private:
    char * begin() { return &*m_buffer.begin();}
    const char * begin() const { return &*m_buffer.begin(); }
    void makeSpace(size_t len)
    {
        // 空闲空间不够
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            m_buffer.resize(m_writeIndex + len);
        }
        /**
         * writableBytes 不够，但是总的空闲空间够，
         * 把数据往前挪，
         * 相当于把 prependableBytes 挪到后面与 writableBytes 拼接
        */
        else 
        {
            size_t readable = readableBytes();
            std::copy(begin() + m_readIndex, begin() + m_writeIndex,
                      begin() + kCheapPrepend);
            m_readIndex = kCheapPrepend;
            m_writeIndex = kCheapPrepend + readable;
        }
    }

private:
    std::vector<char> m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
};

#endif