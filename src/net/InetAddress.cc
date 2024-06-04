#include <strings.h>
#include <string.h>

#include "InetAddress.h"


namespace tinymuduo
{
InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&m_addr, sizeof m_addr);
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof buf);
    return std::string(buf);
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(m_addr.sin_port);
    sprintf(buf+end, ":%u", port);
    return std::string(buf);
}

uint16_t InetAddress::toPort() const
{
    return m_addr.sin_port;
}
} // namespace tinymuduo
