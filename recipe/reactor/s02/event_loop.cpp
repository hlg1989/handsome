//
// Created by gwe on 21-4-23.
//

#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "timer_queue.h"
#include "logging/logger.h"

#include <assert.h>

namespace handsome{

    __thread event_loop* t_loop_in_this_thread = nullptr;

    const int kPollTimeMS = 10000;

    event_loop::event_loop()
        : m_looping(false)
        , m_quit(false)
        , m_thread_id(current_thread::tid())
        , m_poller(new poller(this))
        , m_timer_queue(new timer_queue(this))
    {
        LOG_TRACE << "event_loop created " << this << " in thread " << m_thread_id;
        if(t_loop_in_this_thread){
            LOG_FATAL << "Another event_loop " << t_loop_in_this_thread
                      << " exists in this thread " << m_thread_id;
        }else{
            t_loop_in_this_thread = this;
        }
    }

    event_loop::~event_loop()
    {
        assert(!m_looping);
        t_loop_in_this_thread = nullptr;
    }

    void event_loop::loop()
    {
        assert(!m_looping);
        assert_in_loop_thread();

        m_looping = true;
        m_quit = false;

        while(!m_quit){
            m_active_channels.clear();
            m_poll_return_time = m_poller->poll(kPollTimeMS, &m_active_channels);
            for(auto it = m_active_channels.begin(); it != m_active_channels.end(); ++it){
                (*it)->handle_event();
            }
        }

        LOG_TRACE << "event_loop " << this << " stop looping";
        m_looping = false;
    }

    void event_loop::quit()
    {
        m_quit = true;
    }

    timer_id event_loop::run_at(const timestamp &time, const timer_callback_t &cb)
    {
        return m_timer_queue->add_timer(cb , time, 0.0);
    }

    timer_id event_loop::run_after(double delay, const timer_callback_t &cb)
    {
        timestamp ts = add_time(timestamp::now(), delay);
        return run_at(ts, cb);
    }

    timer_id event_loop::run_every(double interval, const timer_callback_t &cb)
    {
        timestamp ts = add_time(timestamp::now(), interval);
        return m_timer_queue->add_timer(cb, ts, interval);
    }

    void event_loop::update_channel(channel *ch)
    {
        assert(ch->owner_loop() == this);
        assert_in_loop_thread();

        m_poller->update_channel(ch);
    }

    void event_loop::abort_not_in_loop_thread()
    {
        LOG_FATAL << "event_loop::abort_not_in_loop_thread - event_loop " << this
                  << " was created in m_thread_id = " << m_thread_id
                  << ", current thread id = " << current_thread::tid();
    }
}