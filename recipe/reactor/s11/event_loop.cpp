//
// Created by gwe on 21-5-20.
//

#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "timer_queue.h"

#include "logging/logger.h"

#include <functional>
#include <assert.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace handsome{

    __thread event_loop* t_loop_in_this_thread = nullptr;

    const int kPollTimeMS = 10 * 1000;

    static int create_eventfd()
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(fd < 0){
            LOG_SYSFATAL << "Failed in eventfd";
        }

        return fd;
    }

    class ignore_sig_pipe
    {
    public:
        ignore_sig_pipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
        }
    };

    ignore_sig_pipe g_ignore_pipe;



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

        m_wakeup_channel->set_read_callback(std::bind(&event_loop::handle_read, this, std::placeholders::_1));
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
                (*it)->handle_event(m_poll_return_time);
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
            if(cb){
                cb();
            }
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

    timer_id event_loop::run_at(const timestamp &when, const timer_callback_t &cb)
    {
        return m_timer_queue->add_timer(cb, when, 0.0);
    }

    timer_id event_loop::run_after(double delay, const timer_callback_t &cb)
    {
        timestamp when = add_time(timestamp::now(), delay);
        return run_at(when, cb);
    }

    timer_id event_loop::run_every(double interval, const timer_callback_t &cb)
    {
        timestamp when = add_time(timestamp::now(), interval);
        return m_timer_queue->add_timer(cb, when, interval);
    }

    void event_loop::cancel(timer_id tm_id)
    {
        m_timer_queue->cancel(tm_id);
    }

    void event_loop::update_channel(channel *ch)
    {
        assert(ch->own_loop() == this);
        assert_in_loop_thread();

        m_poller->update_channel(ch);
    }

    void event_loop::remove_channel(channel *ch)
    {
        assert(ch->own_loop() == this);
        assert_in_loop_thread();

        m_poller->remove_channel(ch);
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
            LOG_ERROR << "event_loop::wakeup writes " << n << "bytes instead of 8";
        }
    }

    void event_loop::handle_read(timestamp when)
    {
        uint64_t one = 1;
        ssize_t n = ::read(m_wakeup_fd, &one, sizeof(one));
        if(n != sizeof(one)){
            LOG_ERROR << "event_loop::handle_read reads " << n << "bytes instead of 8";
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

        for(auto cb : functors){
            if(cb)
                cb();
        }

        m_calling_pending_functors = false;
    }


}