//
// Created by gwe on 21-5-14.
//

#include "channel.h"
#include "event_loop.h"
#include "logging/logger.h"

#include <sstream>
#include <poll.h>

namespace handsome{

    const int channel::kNoneEvent = 0;
    const int channel::kReadEvent = POLLIN | POLLPRI;
    const int channel::kWriteEvent = POLLOUT;

    channel::channel(event_loop *loop, int fd)
        : m_loop(loop)
        , m_fd(fd)
        , m_events(kNoneEvent)
        , m_revents(kNoneEvent)
        , m_index(-1)
        , m_event_handling(false)
    {

    }

    channel::~channel()
    {
        //assert(!m_event_handling);
    }

    void channel::update()
    {
        m_loop->update_channel(this);
    }

    void channel::handle_event(timestamp receive_time)
    {
        m_event_handling = true;

        if(m_revents & POLLNVAL){
            LOG_WARN << "channel::handle_event POLLNVAL";
        }

        if((m_revents & POLLHUP) && !(m_revents & POLLIN)){
            LOG_WARN << "channel::handle_event POLLHUP";
            if(m_close_cb)
                m_close_cb();
        }

        if(m_revents & (POLLIN | POLLPRI | POLLRDHUP)){
            if(m_read_cb){
                m_read_cb(receive_time);
            }
        }

        if(m_revents & POLLOUT){
            if(m_write_cb)
                m_write_cb();
        }

        m_event_handling = false;
    }

}