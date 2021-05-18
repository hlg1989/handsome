//
// Created by gwe on 21-5-17.
//

#ifndef HANDSOME_TCP_SERVER_H
#define HANDSOME_TCP_SERVER_H

#include "thread/noncopyable.h"
#include "callback.h"
#include "tcp_connection.h"

#include <map>
#include <memory>

namespace handsome{

    class acceptor;
    class event_loop;
    class event_loop_thread_pool;

    class tcp_server : private noncopyable
    {
    public:

        tcp_server(event_loop* loop, const inet_address& listen_addr);

        ~tcp_server();

        void set_thread_num(int num_threads);

        void start();

        void set_connection_callback(const connection_callback_t& cb)
        {
            m_connection_cb = cb;
        }

        void set_message_callback(const message_callback_t& cb)
        {
            m_message_cb = cb;
        }

        void set_write_complete_callback(const write_complete_callback_t& cb)
        {
            m_write_complete_cb = cb;
        }

    private:

        void new_connection(int sockfd, const inet_address& peer_addr);

        void remove_connection(const std::shared_ptr<tcp_connection>& conn);

        void remove_connection_in_loop(const std::shared_ptr<tcp_connection>& conn);

    private:

        typedef std::map<std::string, std::shared_ptr<tcp_connection>> connection_map_t;

        event_loop* m_loop;
        const std::string m_name;
        std::unique_ptr<acceptor> m_acceptor;
        std::unique_ptr<event_loop_thread_pool> m_thread_pool;

        connection_callback_t m_connection_cb;
        message_callback_t m_message_cb;
        write_complete_callback_t m_write_complete_cb;

        bool m_started;
        int m_next_conn_id;
        connection_map_t m_connections;
    };
}


#endif //HANDSOME_TCP_SERVER_H
