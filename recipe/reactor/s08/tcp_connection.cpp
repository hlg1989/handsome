//
// Created by gwe on 21-5-12.
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
        : m_loop(CHECK_NOT_NULL(loop))
        , m_name(name)
        , m_state(kConnecting)
        , m_socket(new socket(sockfd))
        , m_channel(new channel(m_loop, sockfd))
        , m_local_addr(local_addr)
        , m_peer_addr(peer_addr)
    {
        LOG_DEBUG << "tcp_connection::ctor[" << m_name << "] at " << this << " fd = " << sockfd;

        m_channel->set_read_callback(std::bind(&tcp_connection::handle_read, this, std::placeholders::_1));
        m_channel->set_write_callback(std::bind(&tcp_connection::handle_write, this));
        m_channel->set_close_callback(std::bind(&tcp_connection::handle_close, this));
        m_channel->set_error_callback(std::bind(&tcp_connection::handle_error, this));

    }

    tcp_connection::~tcp_connection()
    {
        LOG_DEBUG << "tcp_connection::dtor[" << m_name << "] at " << this << " fd = " << m_channel->fd();
    }

    void tcp_connection::send(const std::string &message)
    {
        if(m_state == kConnected){
            if(m_loop->is_in_loop_thread()){
                send_in_loop(message);
            }else{
                m_loop->run_in_loop(std::bind(&tcp_connection::send_in_loop, this, message));
            }
        }
    }

    void tcp_connection::send_in_loop(const std::string &message)
    {
        m_loop->assert_in_loop_thread();

        ssize_t num_wrote = 0;

        if(!m_channel->is_writing() && m_output_buffer.readable_bytes() == 0){
            num_wrote = ::write(m_channel->fd(), message.data(), message.size());
            if(num_wrote > 0){
                if(implicit_cast<size_t>(num_wrote) < message.size()){
                    LOG_TRACE << "I am going to write more data";
                }
            }else{
                num_wrote = 0;
                if(errno != EWOULDBLOCK){
                    LOG_SYSERR << "tcp_connection::send_in_loop";
                }
            }
        }

        assert(num_wrote >= 0);
        if(implicit_cast<size_t>(num_wrote) < message.size()){
            m_output_buffer.append(message.data() + num_wrote, message.size() - num_wrote);
            if(m_channel->is_writing()){
                m_channel->enable_writing();
            }
        }
    }

    void tcp_connection::shutdown()
    {
        if(m_state == kConnected){
            set_state(kDisconnecting);

            m_loop->run_in_loop(std::bind(&tcp_connection::shutdown_in_loop, this));
        }
    }

    void tcp_connection::shutdown_in_loop()
    {
        m_loop->assert_in_loop_thread();

        if(!m_channel->is_writing()){
            m_socket->shutdown_write();
        }
    }

    void tcp_connection::connection_established()
    {
        m_loop->assert_in_loop_thread();
        assert(m_state == kConnecting);
        set_state(kConnected);

        m_channel->enable_reading();
        m_connection_cb(shared_from_this());
    }

    void tcp_connection::connection_destroyed()
    {
        m_loop->assert_in_loop_thread();
        assert(m_state == kConnected || m_state == kDisconnecting);
        set_state(kDisconnected);
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
        m_loop->assert_in_loop_thread();

        if(m_channel->is_writing()){
            ssize_t n = ::write(m_channel->fd(), m_output_buffer.peek(), m_output_buffer.readable_bytes());

            if(n > 0){
                m_output_buffer.retrieve(n);
                if(m_output_buffer.readable_bytes() == 0){
                    m_channel->disable_writing();
                    if(m_state == kDisconnecting){
                        shutdown_in_loop();
                    }
                }else{
                    LOG_TRACE << "I am going to write more data";
                }
            }else{
                LOG_SYSERR << "tcp_connection::handle_write";
            }
        }else{
            LOG_TRACE << "connection is down, no more writing";
        }
    }

    void tcp_connection::handle_close()
    {
        m_loop->assert_in_loop_thread();

        LOG_TRACE << "tcp_connection::handle_close state = " << m_state;
        assert(m_state == kConnected || m_state == kDisconnecting);

        m_channel->disable_all();
        m_close_cb(shared_from_this());
    }

    void tcp_connection::handle_error()
    {
        int err = socket_ops::get_socket_error(m_channel->fd());
        LOG_ERROR << "tcp_connection::handle_error [" << m_name
                  << "] - SO_ERROR = " << err << " " << strerror_tl(err);
    }


}