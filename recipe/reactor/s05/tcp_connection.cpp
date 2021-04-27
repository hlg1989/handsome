//
// Created by gwe on 21-4-27.
//

#include "tcp_connection.h"
#include "channel.h"
#include "event_loop.h"
#include "socket.h"
#include "logging/logger.h"

#include <functional>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

namespace handsome{

    tcp_connection::tcp_connection(event_loop *loop, const std::string &name, int sockfd,
                                   const inet_address &local_addr, const inet_address &peer_addr)
        : m_loop(loop)
        , m_name(name)
        , m_state(kConnecting)
        , m_socket(new socket(sockfd))
        , m_channel(new channel(m_loop, sockfd))
        , m_local_addr(local_addr)
        , m_peer_addr(peer_addr)
    {
        LOG_DEBUG << "tcp_connection::ctor[" << m_name << "] at " << this
                  << " fd = " << sockfd;

        m_channel->set_read_callback(std::bind(&tcp_connection::handle_read, this));
    }

    tcp_connection::~tcp_connection()
    {
        LOG_DEBUG << "tcp_connection::dtor[" << m_name << "] at" << this
                  << " fd = " << m_channel->fd();
    }

    void tcp_connection::connection_established()
    {
        m_loop->assert_in_loop_thread();
        assert(m_state == kConnecting);
        set_connection_state(kConnected);
        m_channel->enable_reading();

        m_connection_cb(shared_from_this());
    }

    void tcp_connection::handle_read()
    {
        char buf[65536];
        ssize_t n = ::read(m_channel->fd(), buf, sizeof(buf));
        m_message_cb(shared_from_this(), buf, n);

    }
}