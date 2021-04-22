//
// Created by gwe on 21-4-22.
//

#ifndef HANDSOME_EVENT_LOOP_H
#define HANDSOME_EVENT_LOOP_H

#include "thread/noncopyable.h"
#include "thread/thread.h"
#include <memory>
#include <vector>
#include <atomic>

namespace handsome{

    class channel;
    class poller;

    class event_loop : private noncopyable
    {
    public:

        event_loop();
        ~event_loop();

        void loop();

        void quit();

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
        std::unique_ptr<poller> m_poller;
        channel_list_t m_active_channels;
    };
}

#endif //HANDSOME_EVENT_LOOP_H
