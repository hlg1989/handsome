//
// Created by gwe on 21-4-27.
//

#ifndef HANDSOME_TIMER_H
#define HANDSOME_TIMER_H

#include "thread/noncopyable.h"
#include "datetime/timestamp.h"
#include "callback.h"

namespace handsome{

    class timer : private noncopyable
    {
    public:

        timer(const timer_callback_t& cb, timestamp when, double interval)
            : m_callback(cb)
            , m_expiration(when)
            , m_interval(interval)
            , m_repeat(interval > 0.0)
        {

        }

        void run() const
        {
            m_callback();
        }

        timestamp expiration() const
        {
            return m_expiration;
        }

        bool repeat() const
        {
            return m_repeat;
        }

        void restart(timestamp now);

    private:
        const timer_callback_t m_callback;
        timestamp m_expiration;
        const double m_interval;
        const bool m_repeat;
    };
}

#endif //HANDSOME_TIMER_H
