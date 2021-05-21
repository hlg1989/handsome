//
// Created by gwe on 21-5-20.
//

#ifndef HANDSOME_CHANNEL_H
#define HANDSOME_CHANNEL_H

#include "thread/noncopyable.h"
#include "datetime/timestamp.h"

#include <functional>

namespace handsome{

    class event_loop;

    class channel : private noncopyable
    {
    public:

        typedef std::function<void()> event_callback_t;
        typedef std::function<void(timestamp)> read_event_callback_t;

        channel(event_loop* loop, int fd);
        ~channel();

        void handle_event(timestamp receive_time);

        void set_read_callback(const read_event_callback_t& cb)
        {
            m_read_cb = cb;
        }

        void set_write_callback(const event_callback_t& cb)
        {
            m_write_cb = cb;
        }

        void set_error_callback(const event_callback_t& cb)
        {
            m_error_cb = cb;
        }

        void set_close_callback(const event_callback_t& cb)
        {
            m_close_cb = cb;
        }

        int fd() const
        {
            return m_fd;
        }

        int events() const
        {
            return m_events;
        }

        void set_revents(int revents)
        {
            m_revents = revents;
        }

        bool is_none_event() const
        {
            return m_events == kNoneEvent;
        }

        void enable_reading()
        {
            m_events |= kReadEvent;
            update();
        }

        void enable_writing()
        {
            m_events |= kWriteEvent;
            update();
        }

        void disable_writing()
        {
            m_events &= ~kWriteEvent;
            update();
        }

        void disable_all()
        {
            m_events = kNoneEvent;
            update();
        }

        bool is_writing() const
        {
            return m_events & kWriteEvent;
        }

        int index()
        {
            return m_index;
        }

        void set_index(int index)
        {
            m_index = index;
        }

        event_loop* own_loop()
        {
            return m_loop;
        }

    private:

        void update();

    private:

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        event_loop* m_loop;
        const int m_fd;
        int m_events;
        int m_revents;
        int m_index;

        bool m_event_handling;

        read_event_callback_t m_read_cb;
        event_callback_t m_write_cb;
        event_callback_t m_error_cb;
        event_callback_t m_close_cb;

    };
}

#endif //HANDSOME_CHANNEL_H
