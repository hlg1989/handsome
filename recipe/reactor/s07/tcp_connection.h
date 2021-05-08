//
// Created by gwe on 21-4-30.
//

#ifndef HANDSOME_TCP_CONNECTION_H
#define HANDSOME_TCP_CONNECTION_H

#include "buffer.h"
#include "callback.h"
#include "inet_address.h"

#include "thread/noncopyable.h"
#include <memory>

namespace handsome{

    class channel;
    class event_loop;
    class socket;

    class tcp_connection : private noncopyable
                         , public std::enable_shared_from_this<tcp_connection>
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
            return m_state == kConnected;
        }

        void set_connection_callback(const connection_callback_t& cb)
        {
            m_connection_cb = cb;
        }

        void set_message_callback(const message_callback_t& cb)
        {
            m_message_cb = cb;
        }

        void set_close_callback(const close_callback_t& cb)
        {
            m_close_cb = cb;
        }

        void connection_established();

        void connection_destroyed();

    private:

        enum connection_state
        {
            kConnecting,
            kConnected,
            kDisconnected,
        };

        void set_connection_state(connection_state s)
        {
            m_state = s;
        }

        void handle_read(timestamp receive_time);

        void handle_write();

        void handle_close();

        void handle_error();

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
        close_callback_t m_close_cb;
        buffer m_input_buffer;

    };
}

#endif //HANDSOME_TCP_CONNECTION_H
