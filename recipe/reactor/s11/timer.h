//
// Created by gwe on 21-5-20.
//

#ifndef HANDSOME_TIMER_H
#define HANDSOME_TIMER_H

#include "thread/noncopyable.h"
#include "thread/atomic.h"
#include "datetime/timestamp.h"

#include "callback.h"

namespace handsome{

    class timer : private noncopyable
    {
    public:

        timer(const timer_callback_t& cb, timestamp when, double interval)
            : m_cb(cb)
            , m_expiration(when)
            , m_interval(interval)
            , m_repeat(interval > 0.0)
            , m_sequence(s_num_created.increment_and_get())
        {

        }

        void run() const
        {
            if(m_cb)
                m_cb();
        }

        timestamp expiration() const
        {
            return m_expiration;
        }

        bool repeat() const
        {
            return m_repeat;
        }

        int64_t sequence() const
        {
            return m_sequence;
        }

        void restart(timestamp now);

    private:

        const timer_callback_t m_cb;
        timestamp m_expiration;
        const double m_interval;
        const bool m_repeat;
        const int64_t m_sequence;

        static atomic_int64_t s_num_created;
    };
}

#endif //HANDSOME_TIMER_H
