//
// Created by gwe on 21-5-20.
//

#include "event_loop_thread_pool.h"
#include "event_loop_thread.h"
#include "event_loop.h"

#include <functional>

namespace handsome{

    event_loop_thread_pool::event_loop_thread_pool(event_loop *base_loop)
        : m_base_loop(base_loop)
        , m_started(false)
        , m_num_threads(0)
        , m_next(0)
    {

    }

    event_loop_thread_pool::~event_loop_thread_pool()
    {

    }

    void event_loop_thread_pool::start()
    {
        assert(!m_started);
        m_base_loop->assert_in_loop_thread();

        m_started = true;

        for(int i = 0; i < m_num_threads; ++i){
            auto t = std::make_shared<event_loop_thread>();
            m_threads.push_back(t);
            m_loops.push_back(t->start_loop());
        }
    }

    event_loop* event_loop_thread_pool::get_next_loop()
    {
        m_base_loop->assert_in_loop_thread();

        event_loop* loop = m_base_loop;

        if(!m_loops.empty()){
            loop = m_loops[m_next];
            ++m_next;

            if(static_cast<size_t>(m_next) >= m_loops.size()){
                m_next = 0;
            }
        }

        return loop;
    }
}