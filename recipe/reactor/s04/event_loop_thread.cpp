//
// Created by gwe on 21-4-27.
//

#include "event_loop_thread.h"
#include "event_loop.h"

#include <functional>

namespace handsome{

    event_loop_thread::event_loop_thread()
        : m_loop(nullptr)
        , m_exiting(false)
        , m_thread(std::bind(&event_loop_thread::thread_func, this))
        , m_mtx()
        , m_cond(m_mtx)
    {

    }

    event_loop_thread::~event_loop_thread()
    {
        m_exiting = true;
        m_loop->quit();
        m_thread.join();
    }

    event_loop* event_loop_thread::start_loop()
    {
        assert(!m_thread.started());
        m_thread.start();

        {
            mutex_lock_guard lock(m_mtx);
            while(m_loop == nullptr){
                m_cond.wait();
            }
        }

        return m_loop;
    }

    void event_loop_thread::thread_func()
    {
        event_loop loop;

        {
            mutex_lock_guard lock(m_mtx);
            m_loop = &loop;
            m_cond.notify();
        }

        loop.loop();
    }
}