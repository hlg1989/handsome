//
// Created by gwe on 21-4-22.
//

#include "event_loop.h"
#include "channel.h"
#include "poller.h"

#include "logging/logger.h"
#include <assert.h>

namespace handsome
{
    __thread event_loop* t_loop_this_thread = nullptr;
    const int kPollTimeMs = 10000;

    event_loop::event_loop()
        : m_looping(false)
        , m_quit(false)
        , m_thread_id(current_thread::tid())
        , m_poller(new poller(this))
    {
        LOG_TRACE << "event_loop create " << this << " in thread " << m_thread_id;

        if(t_loop_this_thread){
            LOG_FATAL << "Another event_loop " << t_loop_this_thread
                      << " exists in this thread " << m_thread_id;
        }else{
            t_loop_this_thread = this;
        }
    }

    event_loop::~event_loop()
    {
        assert(!m_looping);
        t_loop_this_thread = nullptr;
    }

    void event_loop::loop()
    {
        assert(!m_looping);
        assert_in_loop_thread();
        m_looping = true;
        m_quit = false;

        while(!m_quit){
            m_active_channels.clear();
            m_poller->poll(kPollTimeMs, &m_active_channels);
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