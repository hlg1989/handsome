//
// Created by gwe on 21-4-30.
//

#include "tcp_connection.h"
#include "channel.h"
#include "event_loop.h"
#include "socket.h"
#include "socket_ops.h"
#include "logging/logger.h"

#include <functional>
#include <errno.h>
#include <stdio.h>

namespace handsome{

    tcp_connection::tcp_connection(event_loop *loop, const std::string &name, int sockfd,
                                   const inet_address &local_addr, const inet_address &peer_addr)
        : m_loop(loop)
        , m_name(name)
        , m_state(kConnecting)
        , m_socket(new socket(sockfd))
        , m_channel(new channel(loop, sockfd))
        , m_local_addr(local_addr)
        , m_peer_addr(peer_addr)
    {
        LOG_DEBUG << "tcp_connection:ctor[" << m_name << "] at " << this << " fd = " << sockfd;

        m_channel->set_read_callback(std::bind(&tcp_connection::handle_read, this, std::placeholders::_1));
        m_channel->set_write_callback(std::bind(&tcp_connection::handle_write, this));
        m_channel->set_close_callback(std::bind(&tcp_connection::handle_close, this));
        m_channel->set_error_callback(std::bind(&tcp_connection::handle_error, this));
    }

    tcp_connection::~tcp_connection()
    {
        LOG_DEBUG << "tcp_connection::dtor[" << m_name << "] at " << this << " fd = " << m_channel->fd();
    }

    void tcp_connection::connection_established()
    {
        m_loop->assert_in_loop_thread();
        assert(m_state == kConnecting);
        set_connection_state(kConnected);
        m_channel->enable_reading();
        m_connection_cb(shared_from_this());
    }

    void tcp_connection::connection_destroyed()
    {
        m_loop->assert_in_loop_thread();
        assert(m_state == kConnected);
        set_connection_state(kDisconnected);
        m_channel->disable_all();
        m_connection_cb(shared_from_this());

        m_loop->remove_channel(m_channel.get());
    }

    void tcp_connection::handle_read(timestamp receive_time)
    {
        int save_errno = 0;
        ssize_t n = m_input_buffer.read_fd(m_channel->fd(), &save_errno);
        if(n > 0){
            m_message_cb(shared_from_this(), &m_input_buffer, receive_time);
        }else if(n == 0){
            handle_close();
        }else{
            errno = save_errno;
            LOG_SYSERR << "tcp_connection::handle_read";
            handle_error();
        }
    }

    void tcp_connection::handle_write()
    {

    }

    void tcp_connection::handle_close()
    {
        m_loop->assert_in_loop_thread();
        LOG_TRACE << "tcp_connection::handle_close state = " << m_state;
        assert(m_state == kConnected);
        m_channel->disable_all();
        m_close_cb(shared_from_this());
    }

    void tcp_connection::handle_error()
    {
        int err = socket_ops::get_socket_error(m_channel->fd());
        LOG_ERROR << "tcp_connection::handle_error [" << m_name << "] - SO_ERROR = " << err << " " << strerror_tl(err);
    }
}