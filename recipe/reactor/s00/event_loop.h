//
// Created by gwe on 21-4-22.
//

#ifndef HANDSOME_EVENT_LOOP_H
#define HANDSOME_EVENT_LOOP_H

#include "thread/thread.h"
#include "thread/noncopyable.h"
#include <atomic>

namespace handsome{

    class event_loop : private noncopyable
    {
    public:

        event_loop();
        ~event_loop();

        void loop();

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

        std::atomic_bool m_looping;
        const pid_t m_thread_id;
    };
}


#endif //HANDSOME_EVENT_LOOP_H
