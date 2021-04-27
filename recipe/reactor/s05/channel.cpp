//
// Created by gwe on 21-4-27.
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
    {

    }

    channel::~channel()
    {

    }

    void channel::update()
    {
        m_loop->update_channel(this);
    }

    void channel::handle_event()
    {
        if(m_revents & POLLNVAL){
            LOG_WARN << "channel::handle_event() POLLNVAL";
        }

        if(m_revents & (POLLERR | POLLNVAL)){
            if(m_error_cb)
                m_error_cb();
        }

        if(m_revents & (POLLIN | POLLPRI | POLLRDHUP)){
            if(m_read_cb)
                m_read_cb();
        }

        if(m_revents & POLLOUT){
            if(m_write_cb)
                m_write_cb();
        }
    }
}