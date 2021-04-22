//
// Created by gwe on 21-4-22.
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
        timestamp now = timestamp::now();

        if(num_events > 0){
            LOG_TRACE << num_events << " events happended";
            fill_active_channels(num_events, active_channels);
        }else if(num_events == 0){
            LOG_TRACE << " nothing happened";
        }else{
            LOG_SYSERR << "poller::poll()";
        }

        return now;
    }

    void poller::fill_active_channels(int num_events, channel_list_t *active_channels) const
    {
        for(auto pfd = m_pollfds.begin(); pfd != m_pollfds.end() && num_events > 0; ++pfd){
            if(pfd->revents > 0){
                ++num_events;
                auto ch = m_channels.find(pfd->fd);
                assert(ch != m_channels.end());

                channel* item = ch->second;
                assert(item->fd() == pfd->fd);
                item->set_revents(pfd->revents);

                active_channels->push_back(item);
            }
        }
    }

    void poller::update_channel(channel *ch)
    {
        assert_in_loop_thread();
        LOG_TRACE << "fd = " << ch->fd() << " events = " << ch->events();

        if(ch->index() < 0){
            assert(m_channels.find(ch->fd()) == m_channels.end());
            struct pollfd pfd;
            pfd.fd = ch->fd();
            pfd.events = static_cast<short>(ch->events());
            pfd.revents = 0;
            m_pollfds.push_back(pfd);

            int index = static_cast<int>(m_pollfds.size()) - 1;
            ch->set_index(index);
            m_channels[pfd.fd] = ch;
        }else{
            assert(m_channels.find(ch->fd()) != m_channels.end());
            assert(m_channels[ch->fd()] == ch);
            int index = ch->index();
            assert(0 <= index && index < static_cast<int>(m_pollfds.size()));

            auto& pfd = m_pollfds[index];
            assert(pfd.fd == ch->fd() || pfd.fd == -1);
            pfd.events = static_cast<short>(ch->events());
            pfd.revents = 0;

            if(ch->is_none_event()){
                pfd.fd = -1;
            }
        }
    }
}