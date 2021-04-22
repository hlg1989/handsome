//
// Created by gwe on 21-4-22.
//

#include "event_loop.h"
#include "logging/logger.h"

#include <assert.h>
#include <poll.h>

namespace handsome{

    __thread event_loop* t_loop_in_this_thread = nullptr;

    event_loop::event_loop()
        : m_looping(false)
        , m_thread_id(current_thread::tid())
    {
        LOG_INFO << "event_loop created " << this << " in thread " << m_thread_id;
        if(t_loop_in_this_thread){
            LOG_FATAL << "Another event_loop " << t_loop_in_this_thread << " exists in this thread " << m_thread_id;
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

        ::poll(NULL, 0, 5 * 1000);

        LOG_TRACE << "event_loop " << this << "stop logging";
        m_looping = false;
    }

    void event_loop::abort_not_in_loop_thread()
    {
        LOG_FATAL << "event_loop::abort_not_in_loop_thrad - event_loop " << this
                  << "was create in m_thread_id = " << m_thread_id
                  << ", current thread id = " << current_thread::tid();
    }
}

