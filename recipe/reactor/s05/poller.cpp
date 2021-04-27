//
// Created by gwe on 21-4-27.
//

#include "poller.h"
#include "channel.h"
#include "logging/logger.h"

#include <assert.h>
#include <poll.h>

namespace handsome{

    poller::poller(event_loop *loop)
        : m_loop(loop)
    {

    }

    poller::~poller()
    {

    }

    timestamp poller::poll(int timeout_ms, channel_list_t *active_channels)
    {
        int num_events = ::poll(m_pollfds.data(), m_pollfds.size(), timeout_ms);
        if(num_events > 0){
            LOG_TRACE << num_events << " events hannped";
            fill_active_channels(num_events, active_channels);
        }else if(num_events == 0){
            LOG_TRACE << " nothing happened";
        }else{
            LOG_SYSERR << "poller::poll";
        }

        return timestamp::now();
    }

    void poller::fill_active_channels(int num_events, channel_list_t *active_channels) const
    {
        for(auto it = m_pollfds.begin(); it != m_pollfds.end() && num_events > 0; ++it){
            if(it->revents > 0){

                --num_events;

                auto ch_it = m_channels.find(it->fd);
                assert(ch_it != m_channels.end());

                channel* ch = ch_it->second;
                assert(ch->fd() == it->fd);

                ch->set_revents(it->revents);

                active_channels->push_back(ch);
            }
        }
    }

    void poller::update_channel(channel *ch)
    {
        assert_in_loop_thread();
        LOG_TRACE << " fd = " << ch->fd() << " events = " << ch->events();

        if(ch->index() < 0){

            assert(m_channels.find(ch->fd()) == m_channels.end());
            struct pollfd pfd;
            pfd.fd = ch->fd();
            pfd.events = static_cast<short>(ch->events());
            pfd.revents = 0;
            m_pollfds.push_back(pfd);

            int index = static_cast<int>(m_pollfds.size() - 1);
            ch->set_index(index);

            m_channels[ch->fd()] = ch;
        }else{

            assert(m_channels.find(ch->fd()) != m_channels.end());
            assert(m_channels[ch->fd()] == ch);

            int index = ch->index();
            assert(0 <= index && index < m_pollfds.size());

            struct pollfd& pfd = m_pollfds[ch->index()];
            pfd.events = static_cast<short>(ch->events());
            pfd.revents = 0;

            if(ch->is_none_event()){
                pfd.fd = -1;
            }
        }
    }
}