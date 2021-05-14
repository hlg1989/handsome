//
// Created by gwe on 21-5-13.
//

#ifndef HANDSOME_ACCEPTOR_H
#define HANDSOME_ACCEPTOR_H

#include "channel.h"
#include "socket.h"
#include "thread/noncopyable.h"

#include <functional>

namespace handsome{

    class event_loop;
    class inet_address;

    class acceptor : private noncopyable
    {
    public:

        typedef std::function<void(int sockfd, const inet_address& addr)> new_connection_callback_t;

        acceptor(event_loop* loop, const inet_address& listen_addr);

        void set_new_connection_callback(const new_connection_callback_t& cb)
        {
            m_new_connection_cb = cb;
        }

        bool listening() const
        {
            return m_listening;
        }

        void listen();

    private:

        void handle_read();

    private:

        event_loop* m_loop;
        socket m_accept_socket;
        channel m_accept_channel;
        new_connection_callback_t m_new_connection_cb;
        bool m_listening;
    };
}

#endif //HANDSOME_ACCEPTOR_H
