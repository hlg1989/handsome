//
// Created by gwe on 21-5-13.
//

#include "tcp_server.h"
#include "acceptor.h"
#include "event_loop.h"
#include "socket_ops.h"

#include "logging/logger.h"
#include "../s05/tcp_server.h"
#include <functional>
#include <stdio.h>

namespace handsome{

    tcp_server::tcp_server(event_loop *loop, const inet_address &listen_addr)
        : m_loop(CHECK_NOT_NULL(loop))
        , m_name(listen_addr.to_host_port())
        , m_acceptor(new acceptor(loop, listen_addr))
        , m_started(false)
        , m_next_conn_id(1)
    {
        m_acceptor->set_new_connection_callback(std::bind(&tcp_server::new_connection, this, std::placeholders::_1, std::placeholders::_2));
    }

    tcp_server::~tcp_server()
    {

    }

    void tcp_server::start()
    {
        if(!m_started){
            m_started = true;
        }

        if(!m_acceptor->listening()){
            m_loop->run_in_loop(std::bind(&acceptor::listen, m_acceptor.get()));
        }
    }

    void tcp_server::new_connection(int sockfd, const inet_address &peer_addr)
    {
        m_loop->assert_in_loop_thread();

        char buf[32];
        snprintf(buf, sizeof(buf), "#%d", m_next_conn_id);
        ++m_next_conn_id;

        std::string conn_name = m_name + buf;

        LOG_INFO << "tcp_server::new_connection [" << m_name
                 << "] - new connection [" << conn_name
                 << "] from " << peer_addr.to_host_port();

        inet_address local_addr(socket_ops::get_sockaddr_in(sockfd));

        auto conn = std::make_shared<tcp_connection>(m_loop, conn_name, sockfd, local_addr, peer_addr);
        m_connections[conn_name] = conn;

        conn->set_connection_callback(m_connection_cb);
        conn->set_message_callback(m_message_cb);
        conn->set_write_complete_callback(m_write_complete_cb);
        conn->set_close_callback(std::bind(&tcp_server::remove_connection, this, std::placeholders::_1));

        conn->connection_established();
    }

    void tcp_server::remove_connection(const std::shared_ptr<tcp_connection> &conn)
    {
        m_loop->assert_in_loop_thread();
        LOG_INFO << "tcp_server::remove_connection " << m_name
                 << "] - connection " << conn->name();

        size_t n = m_connections.erase(conn->name());
        assert(n == 1);
        (void)n;

        m_loop->run_in_loop(std::bind(&tcp_connection::connection_destroyed, conn));
    }
}