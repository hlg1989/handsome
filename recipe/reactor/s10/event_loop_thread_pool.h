//
// Created by gwe on 21-5-17.
//

#ifndef HANDSOME_EVENT_LOOP_THREAD_POOL_H
#define HANDSOME_EVENT_LOOP_THREAD_POOL_H

#include "thread/noncopyable.h"
#include "thread/condition.h"
#include "thread/mutex.h"
#include "thread/thread.h"

#include <vector>
#include <memory>
#include <memory>


namespace handsome{

    class event_loop;
    class event_loop_thread;

    class event_loop_thread_pool : private noncopyable
    {
    public:

        event_loop_thread_pool(event_loop* base_loop);

        ~event_loop_thread_pool();

        void set_thread_num(int num_threads)
        {
            m_num_threads = num_threads;
        }

        void start();

        event_loop* get_next_loop();

    private:

        event_loop* m_base_loop;
        bool m_started;
        int m_num_threads;
        int m_next;
        std::vector<std::shared_ptr<event_loop_thread>> m_threads;
        std::vector<event_loop*> m_loops;
    };
}



#endif //HANDSOME_EVENT_LOOP_THREAD_POOL_H
