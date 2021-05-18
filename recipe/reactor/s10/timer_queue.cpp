//
// Created by gwe on 21-5-17.
//

#define __STDC_LIMIT_MACROS

#include "timer_queue.h"
#include "event_loop.h"
#include "timer.h"
#include "timer_id.h"

#include "logging/logger.h"

#include <functional>
#include <sys/timerfd.h>
#include <unistd.h>

namespace handsome{

    namespace detail{

        int create_timerfd()
        {
            int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

            if(fd < 0){
                LOG_SYSFATAL << "Failed in timerfd_create";
            }

            return fd;
        }

        struct timespec how_much_time_from_now(timestamp when)
        {
            int64_t microseconds = when.microseconds_since_epoch() - timestamp::now().microseconds_since_epoch();

            if(microseconds < 100)
                microseconds = 100;

            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(microseconds / timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<long>(microseconds % timestamp::kMicroSecondsPerSecond * 1000);

            return ts;
        }

        void read_timerfd(int timerfd, timestamp now)
        {
            uint64_t how_many;
            ssize_t n = ::read(timerfd, &how_many, sizeof(how_many));

            LOG_TRACE << "timer_queue::handle_read " << how_many << " at " << now.to_string();

            if(n != sizeof(how_many)){
                LOG_ERROR << "timer_queue::handle_read reads " << n << " bytes instead of 8";
            }
        }

        void reset_timerfd(int timerfd, timestamp expiration)
        {
            struct itimerspec new_value;
            struct itimerspec old_value;

            bzero(&new_value, sizeof(new_value));
            bzero(&old_value, sizeof(old_value));

            new_value.it_value = how_much_time_from_now(expiration);

            int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
            if(ret){
                LOG_SYSERR << "timerfd_settime";
            }
        }
    }

    using namespace detail;

    timer_queue::timer_queue(event_loop *loop)
        : m_loop(loop)
        , m_timerfd(create_timerfd())
        , m_timerfd_channel(loop, m_timerfd)
        , m_timers()
    {
        m_timerfd_channel.set_read_callback(std::bind(&timer_queue::handle_read, this));
        m_timerfd_channel.enable_reading();
    }

    timer_queue::~timer_queue()
    {
        ::close(m_timerfd);

        for(auto it = m_timers.begin(); it != m_timers.end(); ++it){
            delete it->second;
        }
    }

    timer_id timer_queue::add_timer(const timer_callback_t &cb, timestamp when, double interval)
    {
        timer* tm = new timer(cb, when, interval);
        m_loop->run_in_loop(std::bind(&timer_queue::add_timer_in_loop, this, tm));

        return timer_id(tm);
    }

    void timer_queue::add_timer_in_loop(timer *tm)
    {
        m_loop->assert_in_loop_thread();

        bool earliest_changed = insert(tm);

        if(earliest_changed){
            reset_timerfd(m_timerfd, tm->expiration());
        }
    }

    void timer_queue::handle_read()
    {
        m_loop->assert_in_loop_thread();

        timestamp now = timestamp::now();

        read_timerfd(m_timerfd, now);

        auto expired = get_expired(now);

        for(auto it = expired.begin(); it != expired.end(); ++it){
            it->second->run();
        }

        reset(expired, now);
    }

    std::vector<timer_queue::entry_t> timer_queue::get_expired(timestamp now)
    {
        std::vector<entry_t> expired;
        entry_t sentry = std::make_pair(now, reinterpret_cast<timer*>(UINTPTR_MAX));
        auto it = m_timers.lower_bound(sentry);
        assert(it == m_timers.end() || now < it->first);

        std::copy(m_timers.begin(), m_timers.end(), std::back_inserter(expired));
        m_timers.erase(m_timers.begin(), it);

        return expired;
    }

    void timer_queue::reset(const std::vector<entry_t> &expired, timestamp now)
    {
        timestamp next_expire;

        for(auto it = expired.begin(); it != expired.end(); ++it){
            if(it->second->repeat()){
                it->second->restart(now);
                insert(it->second);
            }else{
                delete it->second;
            }
        }

        if(!m_timers.empty()){
            next_expire = m_timers.begin()->second->expiration();
        }

        if(next_expire.vaild()){
            reset_timerfd(m_timerfd, next_expire);
        }
    }

    bool timer_queue::insert(timer *tm)
    {
        bool earliest_changed = false;
        timestamp when = tm->expiration();
        auto it = m_timers.begin();
        if(it == m_timers.end() || when < it->first){
            earliest_changed = true;
        }

        auto resut = m_timers.insert(std::make_pair(when, tm));
        return earliest_changed;
    }
}