#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "noncopyable.h"

#include <string>
#include <string.h>
#include<assert.h>

namespace Log
{

namespace detail
{
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 100;

template <int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : m_cur(m_data) { setCookie(cookieStart); }
    ~FixedBuffer() { setCookie(cookieEnd); }
    void append(const char* buf, size_t len)
    {
        // if (implicit)
        if (avail() > len)
        {
            memcpy(m_cur, buf, len);
            m_cur += len;
        }
    }

    const char * data() const { return m_data; }
    int length() const { return static_cast<int>(m_cur - m_data); }

    char* current() { return m_cur; }
    int avail() const { return static_cast<int>(end() - m_cur); }
    void add(size_t len) { m_cur += len; }
    void reset() { m_cur = m_data; }

    const char * debugString();
    void setCookie(void(*cookie)()) { m_cookie = cookie; }
private:
    const char* end() const { return m_data + sizeof m_data; }
    static void cookieStart();
    static void cookieEnd();
    void (*m_cookie)();
    char m_data[SIZE];
    char * m_cur;
};

} // namespace detail


class LogStream : noncopyable
{
    using self = LogStream;
public:
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

    self& operator<<(bool v)
    {
        m_buffer.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v) 
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    self& operator<<(char v)
    {
        m_buffer.append(&v, 1);
        return *this;
    }

    self& operator<<(const char* str)
    {
        if (str)
        {
            m_buffer.append(str, strlen(str));
        }
        else
        {
            m_buffer.append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const unsigned char* str)
    {
        return operator<<((const char*)str);
    }

    self& operator<<(const std::string& v)
    {
        m_buffer.append(v.c_str(), v.size());
        return *this;
    }

    // self& operator<<()

    // LogStream();
    // ~LogStream();

    void append(const char* data, int len) { m_buffer.append(data, len); }
    const Buffer& buffer() const { return m_buffer; }
    void resetBuffer() { m_buffer.reset(); } 

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T);
    // 存放一条消息，LogStream持有，small buffer
    Buffer m_buffer;

    static const int kMaxNumericSize = 48;
};

class Fmt
{
public:
    template<typename T>
    Fmt(const char * fmt, T val);

    const char* data() const { return m_buf; }
    int length() const { return m_length; }

private:
    char m_buf[32];
    int m_length;
};

inline LogStream& operator<<(LogStream& os, const Fmt& fmt)
{
    os.append(fmt.data(), fmt.length());
    return os;
}

std::string formatSI(int64_t n);
std::string formatSI(int64_t n);

} // namespace Log


#endif