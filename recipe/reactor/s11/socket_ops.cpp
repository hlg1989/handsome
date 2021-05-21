//
// Created by gwe on 21-5-20.
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
        return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
    }

    struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
    {
        return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
    }

    void set_non_block_and_close_on_exec(int sockfd)
    {
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);

        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD ,flags);
    }

    int socket_ops::create_non_blocking_or_die()
    {
        int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sockfd < 0){
            LOG_SYSFATAL << "socket_ops::create_non_blocking_or_die";
        }

        set_non_block_and_close_on_exec(sockfd);

        return sockfd;
    }

    int socket_ops::connect(int sockfd, const struct sockaddr_in &addr)
    {
        return ::connect(sockfd, sockaddr_cast(&addr), sizeof(addr));
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
        socklen_t addr_len = sizeof(*addr);

        int connfd = ::accept(sockfd, sockaddr_cast(addr), &addr_len);
        if(connfd < 0){
            int saved_errno = errno;
            LOG_SYSERR << "socket_ops::accept";

            switch (saved_errno){
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
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

        set_non_block_and_close_on_exec(connfd);

        return connfd;
    }

    void socket_ops::close(int sockfd)
    {
        int ret = ::close(sockfd);
        if(ret < 0){
            LOG_SYSFATAL << "socket_ops::close";
        }
    }

    void socket_ops::shutdown_write(int sockfd)
    {
        int ret = ::shutdown(sockfd, SHUT_WR);
        if(ret < 0){
            LOG_SYSFATAL << "socket_ops::shutdown_write";
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
        addr->sin_port = host_to_network16(port);
        if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0){
            LOG_SYSERR << "socket_ops::from_host_port";
        }
    }

    struct sockaddr_in socket_ops::get_local_sockaddr_in(int sockfd)
    {
        struct sockaddr_in local_addr;
        bzero(&local_addr, sizeof(local_addr));
        socklen_t addr_len = sizeof(local_addr);

        if(::getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0){
            LOG_SYSERR << "socket_ops::get_local_sockaddr_in";
        }

        return local_addr;
    }

    struct sockaddr_in socket_ops::get_peer_sockaddr_in(int sockfd)
    {
        struct sockaddr_in peer_addr;
        bzero(&peer_addr, sizeof(peer_addr));
        socklen_t addr_len = sizeof(peer_addr);

        if(::getpeername(sockfd, sockaddr_cast(&peer_addr), &addr_len) < 0){
            LOG_SYSERR << "socket_ops::get_peer_sockaddr_in";
        }

        return peer_addr;
    }

    int socket_ops::get_socket_error(int sockfd)
    {
        int opt_val;
        socklen_t opt_len = sizeof(opt_val);

        if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &opt_val, &opt_len) < 0){
            return errno;
        }else{
            return opt_val;
        }
    }

    bool socket_ops::is_self_connect(int sockfd)
    {
        struct sockaddr_in local_addr = get_local_sockaddr_in(sockfd);
        struct sockaddr_in peer_addr = get_peer_sockaddr_in(sockfd);

        return local_addr.sin_port == peer_addr.sin_port && local_addr.sin_addr.s_addr == peer_addr.sin_addr.s_addr;
    }


}