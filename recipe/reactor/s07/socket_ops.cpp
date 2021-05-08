//
// Created by gwe on 21-4-30.
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

    void set_non_blocking_and_close_on_exec(int sockfd)
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

        set_non_blocking_and_close_on_exec(sockfd);

        return sockfd;
    }

    void socket_ops::bind_or_die(int sockfd, const struct sockaddr_in &addr)
    {
        socklen_t len = sizeof(addr);
        int ret = bind(sockfd, sockaddr_cast(&addr), len);
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

    int socket_ops::accept(int sockfd, struct sockaddr_in *peer_addr)
    {
        socklen_t len = sizeof(*peer_addr);

        int connfd = ::accept(sockfd, sockaddr_cast(peer_addr), &len);
        if(connfd < 0){
            LOG_SYSFATAL << "socket_ops::accept";
        }

        set_non_blocking_and_close_on_exec(connfd);

        return connfd;
    }

    void socket_ops::close(int sockfd)
    {
        int ret = ::close(sockfd);
        if(ret < 0){
            LOG_SYSFATAL << "socket_ops::close";
        }
    }

    void socket_ops::to_host_port(char *buf, size_t len, const struct sockaddr_in &addr)
    {
        char host[INET_ADDRSTRLEN] = "invalid";
        ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
        uint16_t port = network_to_host16(addr.sin_port);
        snprintf(buf, len, "%s:%d", host, port);
    }

    void socket_ops::from_host_port(const char *ip, uint16_t port, struct sockaddr_in *addr)
    {
        addr->sin_port = host_to_network16(port);
        addr->sin_family = AF_INET;
        if(::inet_pton(AF_INET, ip, &addr->sin_addr) < 0){
            LOG_SYSERR << "socket_ops::from_host_port";
        }
    }

    struct sockaddr_in socket_ops::get_local_address(int sockfd)
    {
        struct sockaddr_in local_addr;
        bzero(&local_addr, sizeof(local_addr));
        socklen_t addr_len = sizeof(local_addr);

        if(::getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0){
            LOG_SYSERR << "socket_ops::get_local_address";
        }

        return local_addr;
    }

    int socket_ops::get_socket_error(int sockfd)
    {
        int opt_val;
        socklen_t len = sizeof(opt_val);

        if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &opt_val, &len) < 0){
            return errno;
        }else{
            return opt_val;
        }
    }


}