//
// Created by gwe on 21-4-27.
//

#include "event_loop.h"
#include "poller.h"
#include "timer_queue.h"
#include "logging/logger.h"

#include <functional>
#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace handsome{

    __thread event_loop* t_loop_in_this_thread = nullptr;

    const int kPollTimeMS = 10 * 1000;

    static int create_eventfd()
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(fd < 0){
            LOG_SYSERR << "Failed in eventfd";
        }

        return fd;
    }

    event_loop::event_loop()
        : m_looping(false)
        , m_quit(false)
        , m_calling_pending_functors(false)
        , m_thread_id(current_thread::tid())
        , m_poller(new poller(this))
        , m_timer_queue(new timer_queue(this))
        , m_wakeup_fd(create_eventfd())
        , m_wakeup_channel(new channel(this, m_wakeup_fd))
    {
        LOG_TRACE << "event_loop created " << this << " in thread " << m_thread_id;
        if(t_loop_in_this_thread){
            LOG_FATAL << "another event_loop " << t_loop_in_this_thread
                      << " exists in this thread " << m_thread_id;
        }else{
            t_loop_in_this_thread = this;
        }

        m_wakeup_channel->set_read_callback(std::bind(&event_loop::handle_read, this));
        m_wakeup_channel->enable_reading();

    }

    event_loop::~event_loop()
    {
        assert(!m_looping);
        ::close(m_wakeup_fd);
        t_loop_in_this_thread = nullptr;
    }

    void event_loop::loop()
    {
        assert(!m_looping);
        assert_in_loop_thread();

        m_looping = true;
        m_quit = false;

        while(!m_quit){
            m_active_channels.clear();
            m_poll_return_time = m_poller->poll(kPollTimeMS, &m_active_channels);
            for(auto it = m_active_channels.begin(); it != m_active_channels.end(); ++it){
                (*it)->handle_event();
            }

            do_pending_functors();
        }

        LOG_TRACE << "event_loop " << this << " stop looping";
        m_looping = false;
    }

    void event_loop::quit()
    {
        m_quit = true;
        if(!is_in_loop_thread()){
            wakeup();
        }
    }

    void event_loop::run_in_loop(const functor_t &cb)
    {
        if(is_in_loop_thread()){
            cb();
        }else{
            queue_in_loop(cb);
        }
    }

    void event_loop::queue_in_loop(const functor_t &cb)
    {
        {
            mutex_lock_guard lock(m_mtx);
            m_pending_functors.push_back(cb);
        }

        if(!is_in_loop_thread() || m_calling_pending_functors){
            wakeup();
        }
    }

    timer_id event_loop::run_at(const timestamp &time, const timer_callback_t &cb)
    {
        return m_timer_queue->add_timer(cb, time, 0.0);
    }

    timer_id event_loop::run_after(double delay, const timer_callback_t &cb)
    {
        timestamp time = add_time(timestamp::now(), delay);
        return m_timer_queue->add_timer(cb, time, 0.0);
    }

    timer_id event_loop::run_every(double interval, const timer_callback_t &cb)
    {
        timestamp time = add_time(timestamp::now(), interval);
        return m_timer_queue->add_timer(cb, time, interval);
    }

    void event_loop::update_channel(channel *ch)
    {
        assert(ch->owner_loop() == this);
        assert_in_loop_thread();

        m_poller->update_channel(ch);
    }

    void event_loop::abort_not_in_loop_thread()
    {
        LOG_FATAL << "event_loop::abort_not_in_loop_thread - event_loop " << this
                  << " was created in m_thread_id = " << m_thread_id
                  << ", current thread id = " << current_thread::tid();
    }

    void event_loop::wakeup()
    {
        uint64_t one = 1;
        ssize_t n = ::write(m_wakeup_fd, &one, sizeof(one));
        if(n != sizeof(one)){
            LOG_ERROR << "event_loop::wakeup() writes " << n << " bytes instead of 8 bytes";
        }
    }

    void event_loop::handle_read()
    {
        uint64_t one = 1;
        ssize_t n = ::read(m_wakeup_fd, &one, sizeof(one));
        if(n != sizeof(one)){
            LOG_ERROR << "event_loop::handle_read() read " << n << " bytes instead of 8 bytes";
        }

    }

    void event_loop::do_pending_functors()
    {
        std::vector<functor_t> functors;
        m_calling_pending_functors = true;

        {
            mutex_lock_guard lock(m_mtx);
            functors.swap(m_pending_functors);
        }

        for(auto& cb : functors){
            cb();
        }

        m_calling_pending_functors = false;
    }


}