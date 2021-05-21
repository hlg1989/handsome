//
// Created by gwe on 21-5-20.
//

#ifndef HANDSOME_EVENT_LOOP_THREAD_H
#define HANDSOME_EVENT_LOOP_THREAD_H

#include "thread/noncopyable.h"
#include "thread/mutex.h"
#include "thread/condition.h"
#include "thread/thread.h"

namespace handsome{

    class event_loop;

    class event_loop_thread : private noncopyable
    {
    public:

        event_loop_thread();

        ~event_loop_thread();

        event_loop* start_loop();

    private:

        void thread_func();

    private:

        event_loop* m_loop;
        bool m_exiting;
        thread m_thread;
        mutex m_mtx;
        condition m_cond;
    };
}

#endif //HANDSOME_EVENT_LOOP_THREAD_H
