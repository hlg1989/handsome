//
// Created by gwe on 21-4-27.
//

#ifndef HANDSOME_TCP_CONNECTION_H
#define HANDSOME_TCP_CONNECTION_H

#include "callback.h"
#include "inet_address.h"
#include "thread/noncopyable.h"

#include <memory>

namespace handsome{

    class channel;
    class event_loop;
    class socket;

    class tcp_connection : public std::enable_shared_from_this<tcp_connection>
                         , private noncopyable
    {
    public:

        tcp_connection(event_loop* loop, const std::string& name, int sockfd, const inet_address& local_addr, const inet_address& peer_addr);

        ~tcp_connection();

        event_loop* get_loop() const
        {
            return m_loop;
        }

        const std::string& name() const
        {
            return m_name;
        }

        const inet_address& local_address()
        {
            return m_local_addr;
        }

        const inet_address& peer_address()
        {
            return m_peer_addr;
        }

        bool connected() const
        {
            m_state == kConnected;
        }

        void set_connectin_callback(const connection_callback_t& cb)
        {
            m_connection_cb = cb;
        }

        void set_message_callback(const message_callback_t& cb)
        {
            m_message_cb = cb;
        }

        void connection_established();


    private:
        enum connection_state
        {
            kConnecting,
            kConnected,
        };

        void set_connection_state(connection_state s)
        {
            m_state = s;
        }

        void handle_read();

    private:

        event_loop* m_loop;
        std::string m_name;
        connection_state m_state;

        std::unique_ptr<socket> m_socket;
        std::unique_ptr<channel> m_channel;
        inet_address m_local_addr;
        inet_address m_peer_addr;

        connection_callback_t m_connection_cb;
        message_callback_t m_message_cb;
    };
}

#endif //HANDSOME_TCP_CONNECTION_H
