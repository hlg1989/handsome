//
// Created by gwe on 21-5-21.
//

#ifndef HANDSOME_CONNECTOR_H
#define HANDSOME_CONNECTOR_H

#include "inet_address.h"
#include "timer_id.h"
#include "thread/noncopyable.h"

#include <memory>
#include <functional>
#include <atomic>

namespace handsome{

    class channel;
    class event_loop;

    class connector : private noncopyable
    {
    public:

        typedef std::function<void(int sockfd)> new_connection_callback_t;

        connector(event_loop* loop, const inet_address& server_addr);

        ~connector();

        void set_new_connection_callback(const new_connection_callback_t& cb)
        {
            m_new_connection_cb = cb;
        }

        void start();

        void restart();

        void stop();

        const inet_address& server_address() const
        {
            return m_server_addr;
        }

    private:

        enum connection_states{
            kDisconnected,
            kConnecting,
            kConnected
        };

        static const int kMaxRetryDelayMS = 30 * 1000;
        static const int kInitRetryDelayMS = 500;

        void set_state(connection_states s)
        {
            m_state = s;
        }

        void start_in_loop();

        void connect();

        void connecting(int sockfd);

        void handle_write();

        void handle_error();

        void retry(int sockfd);

        int remove_and_reset_channel();

        void reset_channel();

    private:

        event_loop* m_loop;
        inet_address m_server_addr;
        std::atomic_bool m_connect;
        std::atomic<connection_states> m_state;

        std::unique_ptr<channel> m_channel;

        new_connection_callback_t m_new_connection_cb;
        int m_retry_delay_ms;
        timer_id m_timer_id;
    };
}

#endif //HANDSOME_CONNECTOR_H
