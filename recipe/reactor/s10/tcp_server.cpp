//
// Created by gwe on 21-5-17.
//

#include "tcp_server.h"
#include "acceptor.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "socket_ops.h"

#include "logging/logger.h"

#include <functional>
#include <stdio.h>

namespace handsome{

    tcp_server::tcp_server(event_loop *loop, const inet_address &listen_addr)
        : m_loop(CHECK_NOT_NULL(loop))
        , m_name(listen_addr.to_host_port())
        , m_acceptor(new acceptor(loop, listen_addr))
        , m_thread_pool(new event_loop_thread_pool(loop))
        , m_started(false)
        , m_next_conn_id(1)
    {
        m_acceptor->set_new_connection_callback(std::bind(&tcp_server::new_connection, this, std::placeholders::_1, std::placeholders::_2));
    }

    tcp_server::~tcp_server()
    {

    }

    void tcp_server::set_thread_num(int num_threads)
    {
        assert(num_threads >= 0);

        m_thread_pool->set_thread_num(num_threads);
    }

    void tcp_server::start()
    {
        if(!m_started){
            m_started = true;
            m_thread_pool->start();
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

        inet_address local_addr = socket_ops::get_local_sockaddr_in(sockfd);

        event_loop* io_loop = m_thread_pool->get_next_loop();

        auto conn = std::make_shared<tcp_connection>(io_loop, conn_name, sockfd, local_addr, peer_addr);

        m_connections[conn_name] = conn;

        conn->set_connection_callback(m_connection_cb);
        conn->set_message_callback(m_message_cb);
        conn->set_write_complete_callback(m_write_complete_cb);
        conn->set_close_callback(std::bind(&tcp_server::remove_connection, this, std::placeholders::_1));

        io_loop->run_in_loop(std::bind(&tcp_connection::connection_established, conn));
    }

    void tcp_server::remove_connection(const std::shared_ptr<tcp_connection> &conn)
    {
        m_loop->run_in_loop(std::bind(&tcp_server::remove_connection_in_loop, this, conn));
    }

    void tcp_server::remove_connection_in_loop(const std::shared_ptr<tcp_connection> &conn)
    {
        m_loop->assert_in_loop_thread();
        LOG_INFO << "tcp_server::remove_connection_in_loop [" << m_name
                 << "] - connection " << conn->name();

        size_t n = m_connections.erase(conn->name());
        assert(n == 1);
        (void)n;

        event_loop* io_loop = conn->get_loop();
        io_loop->queue_in_loop(std::bind(&tcp_connection::connection_destroyed, conn));
    }
}