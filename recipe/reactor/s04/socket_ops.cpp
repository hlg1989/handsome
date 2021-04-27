//
// Created by gwe on 21-4-25.
//

#include "socket_ops.h"
#include "logging/logger.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace handsome{

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
    {
        return static_cast<const sockaddr*>(implicit_cast<const void*>(addr));
    }

    struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
    {
        return static_cast<sockaddr*>(implicit_cast<void*>(addr));
    }

    void set_non_blcok_and_close_on_exec(int sockfd)
    {
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);

        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
    }

    int socket_ops::create_non_blocking_or_die()
    {
        int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sockfd < 0){
            LOG_SYSFATAL << "socket_ops::create_non_blocking_or_die";
        }

        set_non_blcok_and_close_on_exec(sockfd);

        return sockfd;
    }

    void socket_ops::bind_or_die(int sockfd, const struct sockaddr_in &addr)
    {
        int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
        if(ret < 0){
            LOG_SYSFATAL << "socket_ops::bind_or_die";
        }
    }

    void socket_ops::listen_or_die(int sockfd)
    {
        int ret = ::listen(sockfd, SOMAXCONN);
        if(ret < 0){
            LOG_SYSFATAL << "socket_ops::listen_or_die";
        }
    }

    int socket_ops::accept(int sockfd, struct sockaddr_in *addr)
    {
        socklen_t addrlen = sizeof(*addr);

        int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
        set_non_blcok_and_close_on_exec(connfd);

        if(connfd < 0){
            int saved_errno = errno;
            LOG_SYSERR << "socket_ops::accept";

            switch(saved_errno)
            {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO:
                case EPERM:
                case EMFILE:
                    errno = saved_errno;
                    break;

                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                    LOG_FATAL << "unexpected error of ::accept " << saved_errno;
                    break;

                default:
                    LOG_FATAL << "unknown error of ::accept " << saved_errno;
                    break;
            }
        }

        return connfd;
    }

    void socket_ops::close(int sockfd)
    {
        if(::close(sockfd) < 0){
            LOG_SYSERR << " socket_ops::close";
        }
    }

    void socket_ops::to_host_port(char *buf, size_t size, const struct sockaddr_in &addr)
    {
        char host[INET_ADDRSTRLEN] = "INVALID";
        ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
        uint16_t port = socket_ops::network_to_host16(addr.sin_port);
        snprintf(buf, size, "%s:%u", host, port);
    }

    void socket_ops::from_host_port(const char *ip, uint16_t port, struct sockaddr_in *addr)
    {
        addr->sin_family = AF_INET;
        addr->sin_port = socket_ops::host_to_network16(port);
        if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0){
            LOG_SYSERR << "socket_ops::from_host_port";
        }
    }
}