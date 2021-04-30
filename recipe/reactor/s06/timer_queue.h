//
// Created by gwe on 21-4-28.
//

#ifndef HANDSOME_TIMER_QUEUE_H
#define HANDSOME_TIMER_QUEUE_H

#include "thread/noncopyable.h"
#include "thread/mutex.h"
#include "datetime/timestamp.h"
#include "callback.h"
#include "channel.h"

#include <set>
#include <vector>
#include <utility>

namespace handsome{

    class event_loop;
    class timer;
    class timer_id;

    class timer_queue : private noncopyable
    {
    public:

        timer_queue(event_loop* loop);
        ~timer_queue();

        timer_id add_timer(const timer_callback_t& cb, timestamp when, double interval);

    private:

        typedef std::pair<timestamp, timer*> entry_t;
        typedef std::set<entry_t> timer_list_t;

        void add_timer_in_loop(timer* tm);

        void handle_read();

        std::vector<entry_t> get_expired(timestamp now);

        void reset(const std::vector<entry_t>& expired, timestamp now);

        bool insert(timer* tm);

    private:

        event_loop* m_loop;
        const int m_timerfd;
        channel m_timerfd_channel;
        timer_list_t m_timers;
    };
}

#endif //HANDSOME_TIMER_QUEUE_H
