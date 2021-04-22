//
// Created by gwe on 21-4-22.
//

#ifndef HANDSOME_CHANNEL_H
#define HANDSOME_CHANNEL_H

#include "thread/noncopyable.h"
#include <functional>
#include <memory>

namespace handsome{

    class event_loop;

    class channel : private noncopyable
    {
    public:
        typedef std::function<void()> event_callback_t;

        channel(event_loop* loop, int fd);

        void handle_event();

        void set_read_callback(const event_callback_t& cb)
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

        int fd() const
        {
            return m_fd;
        }

        int events() const
        {
            return m_events;
        }

        int set_revents(int revent)
        {
            m_revents = revent;
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

        void disable_write()
        {
            m_events &= ~kWriteEvent;
            update();
        }

        void disable_all()
        {
            m_events &= kNoneEvent;
            update();
        }

        int index()
        {
            return m_index;
        }

        void set_index(int index)
        {
            m_index = index;
        }

        event_loop* owner_loop()
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

        event_callback_t m_read_cb;
        event_callback_t m_write_cb;
        event_callback_t m_error_cb;

    };

}


#endif //HANDSOME_CHANNEL_H