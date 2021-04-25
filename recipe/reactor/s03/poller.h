//
// Created by gwe on 21-4-25.
//

#ifndef HANDSOME_POLLER_H
#define HANDSOME_POLLER_H

#include "datetime/timestamp.h"
#include "thread/noncopyable.h"
#include "event_loop.h"
#include <vector>
#include <map>

struct pollfd;

namespace handsome{

    class channel;

    class poller : private noncopyable
    {
    public:
        typedef std::vector<channel*> channel_list_t;

        poller(event_loop* loop);
        ~poller();

        timestamp poll(int timeout_ms, channel_list_t* active_channels);

        void update_channel(channel* ch);

        void assert_in_loop_thread()
        {
            m_loop->assert_in_loop_thread();
        }

    private:

        void fill_active_channels(int num_events, channel_list_t* active_channels) const;

    private:

        typedef std::vector<struct pollfd> pollfd_list_t;
        typedef std::map<int, channel*> channel_map_t;

        event_loop* m_loop;
        pollfd_list_t m_pollfds;
        channel_map_t  m_channels;
    };
}

#endif //HANDSOME_POLLER_H
