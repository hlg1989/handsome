//
// Created by gwe on 21-5-17.
//

#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket_ops.h"
#include "logging/logger.h"

#include <functional>

namespace handsome{

    acceptor::acceptor(event_loop *loop, const inet_address &listen_addr)
        : m_loop(loop)
        , m_accept_socket(socket_ops::create_non_blocking_or_die())
        , m_accept_channel(loop, m_accept_socket.fd())
        , m_listening(false)
    {
        m_accept_socket.set_reuse_address(true);
        m_accept_socket.bind_address(listen_addr);

        m_accept_channel.set_read_callback(std::bind(&acceptor::handle_read, this));
    }

    acceptor::~acceptor()
    {

    }

    void acceptor::listen()
    {
        m_loop->assert_in_loop_thread();

        m_listening = true;
        m_accept_socket.listen();
        m_accept_channel.enable_reading();
    }

    void acceptor::handle_read()
    {
        m_loop->assert_in_loop_thread();

        inet_address peer_address(0);

        int connfd = m_accept_socket.accept(&peer_address);
        if(connfd >= 0){
            if(m_new_connection_cb){
                m_new_connection_cb(connfd, peer_address);
            }else{
                socket_ops::close(connfd);
            }
        }
    }
}