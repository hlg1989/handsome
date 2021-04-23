//
// Created by gwe on 21-4-23.
//

#ifndef HANDSOME_EVENT_LOOP_H
#define HANDSOME_EVENT_LOOP_H

#include "datetime/timestamp.h"
#include "thread/thread.h"
#include "thread/noncopyable.h"
#include "callback.h"
#include "timer_id.h"

#include <memory>
#include <vector>
#include <atomic>

namespace handsome{

    class channel;
    class poller;
    class timer_queue;

    class event_loop : private noncopyable
    {
    public:
        event_loop();

        ~event_loop();

        void loop();

        void quit();

        timestamp poll_return_time() const
        {
            return m_poll_return_time;
        };

        timer_id run_at(const timestamp& time, const timer_callback_t& cb);

        timer_id run_after(double delay, const timer_callback_t& cb);

        timer_id run_every(double interval, const timer_callback_t& cb);

        void update_channel(channel* ch);

        void assert_in_loop_thread()
        {
            if(!is_in_loop_thread()){
                abort_not_in_loop_thread();
            }
        }

        bool is_in_loop_thread() const
        {
            return m_thread_id == current_thread::tid();
        }

    private:

        void abort_not_in_loop_thread();

    private:

        typedef std::vector<channel*> channel_list_t;

        std::atomic_bool m_looping;
        std::atomic_bool m_quit;
        const pid_t m_thread_id;
        timestamp m_poll_return_time;
        std::unique_ptr<poller> m_poller;
        std::unique_ptr<timer_queue> m_timer_queue;
        channel_list_t m_active_channels;
    };
}

#endif //HANDSOME_EVENT_LOOP_H
