#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>

#include "MuduoSocket.h"
#include "Logging.h"

namespace tinymuduo
{
muduoSocket::~muduoSocket()
{
    ::close(m_sockfd);
}

void muduoSocket::bindAddress(const InetAddress & localaddr)
{
    if (0 != ::bind(m_sockfd, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)))
    {
        LOG_FATAL << "bind sockfd " << m_sockfd << " error";
    }
}
void muduoSocket::linsten()
{
    if (0 != ::listen(m_sockfd, 1024))
    {
        LOG_FATAL << "listen sockfd " << m_sockfd << " error";
    }
}

int muduoSocket::accept(InetAddress * peeraddr)
{
    sockaddr_in addr;
    ::bzero(&addr, sizeof addr);
    socklen_t len = sizeof addr;
    int connfd = ::accept4(m_sockfd, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd > 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void muduoSocket::shutdownWrite()
{
    if (::shutdown(m_sockfd, SHUT_WR) < 0)
    {
        LOG_ERROR << "shutdownWrite error";
    }
}

void muduoSocket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void muduoSocket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void muduoSocket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void muduoSocket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}
} // namespace tinymuduo
