//
// Created by gwe on 21-5-11.
//

#include "socket.h"
#include "socket_ops.h"
#include "inet_address.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>

namespace handsome{

    socket::~socket()
    {
        socket_ops::close(m_sockfd);
    }

    void socket::bind_address(const inet_address &local_addr)
    {
        socket_ops::bind_or_die(m_sockfd, local_addr.get_sockaddr_in());
    }

    void socket::listen()
    {
        socket_ops::listen_or_die(m_sockfd);
    }

    int socket::accept(inet_address *peer_addr)
    {
        struct sockaddr_in addr;
        bzero(&addr, sizeof(addr));
        int connfd = socket_ops::accept(m_sockfd, &addr);
        if(connfd >= 0){
            peer_addr->set_sockaddr_in(addr);
        }

        return connfd;
    }

    void socket::set_reuse_address(bool on)
    {
        int opt_val = on ? 1 : 0;
        ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
    }

    void socket::shutdown_write()
    {
        socket_ops::shutdown_write(m_sockfd);
    }
}