#include <time.h>

#include "timestamp.h"

Timestamp::Timestamp() : m_microSecondsSinceEpoch(0)
{

}
Timestamp::Timestamp(int64_t microSecondsSinceEpoch) : m_microSecondsSinceEpoch(microSecondsSinceEpoch)
{

}
Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}
std::string Timestamp::toString() const
{
    tm * tm_time = localtime(&m_microSecondsSinceEpoch);
    char buf[128] = {0};
    snprintf(buf, 128, "%4d/%02d/%02d > %02d:%02d:%02d", 
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return std::string(buf);
}

