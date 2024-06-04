#ifndef MUODUOSOCKET_H
#define MUODUOSOCKET_H

#include <netinet/tcp.h>

#include "InetAddress.h"
#include "noncopyable.h"

namespace tinymuduo
{
class muduoSocket : noncopyable
{
public:
    explicit muduoSocket(int sockfd) :
        m_sockfd(sockfd)
    { }
    ~muduoSocket();

    int fd() const { return m_sockfd; }
    
    void bindAddress(const InetAddress & localaddr);
    void linsten();
    int accept(InetAddress * peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);
private:
    const int m_sockfd;
};
} // namespace tinymuduo



#endif