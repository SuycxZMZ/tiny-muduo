// #include <sys/epoll.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <errno.h>

// #define MAX_EVENTS 10
// #define READ_BUFFER_SIZE 1024
// #define PORT 8080

// void set_nonblocking(int sockfd) {
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
// }

// int main() {
//     int listen_fd, conn_fd, nfds, epoll_fd;
//     struct epoll_event ev, events[MAX_EVENTS];
//     struct sockaddr_in addr;
//     char buffer[READ_BUFFER_SIZE];
    
//     listen_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (listen_fd == -1) {
//         perror("socket");
//         exit(EXIT_FAILURE);
//     }

//     set_nonblocking(listen_fd);

//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = INADDR_ANY;
//     addr.sin_port = htons(PORT);

//     if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//         perror("bind");
//         close(listen_fd);
//         exit(EXIT_FAILURE);
//     }

//     if (listen(listen_fd, SOMAXCONN) == -1) {
//         perror("listen");
//         close(listen_fd);
//         exit(EXIT_FAILURE);
//     }

//     epoll_fd = epoll_create1(0);
//     if (epoll_fd == -1) {
//         perror("epoll_create1");
//         close(listen_fd);
//         exit(EXIT_FAILURE);
//     }

//     ev.events = EPOLLIN;
//     ev.data.fd = listen_fd;
//     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
//         perror("epoll_ctl: listen_fd");
//         close(listen_fd);
//         close(epoll_fd);
//         exit(EXIT_FAILURE);
//     }

//     for (;;) {
//         nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
//         if (nfds == -1) {
//             perror("epoll_wait");
//             close(listen_fd);
//             close(epoll_fd);
//             exit(EXIT_FAILURE);
//         }

//         for (int n = 0; n < nfds; ++n) {
//             if (events[n].data.fd == listen_fd) {
//                 // New incoming connection
//                 while ((conn_fd = accept(listen_fd, NULL, NULL)) != -1) {
//                     set_nonblocking(conn_fd);
//                     ev.events = EPOLLIN | EPOLLET;
//                     ev.data.fd = conn_fd;
//                     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
//                         perror("epoll_ctl: conn_fd");
//                         close(conn_fd);
//                     }
//                 }
//                 if (errno != EAGAIN && errno != EWOULDBLOCK) {
//                     perror("accept");
//                 }
//             } else {
//                 // Handle client connection
//                 int client_fd = events[n].data.fd;
//                 if (events[n].events & EPOLLIN) {
//                     ssize_t count;
//                     while ((count = read(client_fd, buffer, READ_BUFFER_SIZE)) > 0) {
//                         // Echo the data back to the client
//                         ssize_t sent = 0;
//                         while (sent < count) {
//                             ssize_t write_count = write(client_fd, buffer + sent, count - sent);
//                             if (write_count == -1) {
//                                 perror("write");
//                                 close(client_fd);
//                                 break;
//                             }
//                             sent += write_count;
//                         }
//                     }
//                     if (count == -1 && errno != EAGAIN) {
//                         perror("read");
//                         close(client_fd);
//                     }
//                     if (count == 0) {
//                         // Client disconnected
//                         close(client_fd);
//                     }
//                 }
//             }
//         }
//     }

//     close(listen_fd);
//     close(epoll_fd);
//     return 0;
// }
// ------------------------------------ httptest ------------------------------------
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_EVENTS 10
#define READ_BUFFER_SIZE 1024
#define PORT 8080

void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd, conn_fd, nfds, epoll_fd;
    struct epoll_event ev, events[MAX_EVENTS];
    struct sockaddr_in addr;
    char buffer[READ_BUFFER_SIZE];
    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(listen_fd);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        perror("epoll_ctl: listen_fd");
        close(listen_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    for (;;) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            close(listen_fd);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_fd) {
                // New incoming connection
                while ((conn_fd = accept(listen_fd, NULL, NULL)) != -1) {
                    set_nonblocking(conn_fd);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
                        perror("epoll_ctl: conn_fd");
                        close(conn_fd);
                    }
                    printf("Accepted new connection: fd %d\n", conn_fd);
                }
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept");
                }
            } else {
                // Handle client connection
                int client_fd = events[n].data.fd;
                if (events[n].events & EPOLLIN) {
                    ssize_t count;
                    while ((count = read(client_fd, buffer, READ_BUFFER_SIZE)) > 0) {
                        printf("Received %zd bytes from fd %d\n", count, client_fd);
                        const char* http_response = "HTTP/1.1 200 OK\r\nContent-Length: ";
                        char response_header[256];
                        snprintf(response_header, sizeof(response_header), "%s%ld\r\n\r\n", http_response, count);
                        write(client_fd, response_header, strlen(response_header));
                        ssize_t sent = 0;
                        while (sent < count) {
                            ssize_t write_count = write(client_fd, buffer + sent, count - sent);
                            if (write_count == -1) {
                                perror("write");
                                close(client_fd);
                                break;
                            }
                            sent += write_count;
                        }
                        printf("Echoed %zd bytes to fd %d\n", count, client_fd);
                    }
                    if (count == -1 && errno != EAGAIN) {
                        perror("read");
                        close(client_fd);
                    }
                    if (count == 0) {
                        // Client disconnected
                        printf("Client fd %d disconnected\n", client_fd);
                        close(client_fd);
                    }
                }
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}

