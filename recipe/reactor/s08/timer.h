//
// Created by gwe on 21-5-10.
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
            : m_cb(cb)
            , m_expiration(when)
            , m_interval(interval)
            , m_repeat(interval > 0.0)
        {

        }

        void run() const
        {
            if(m_cb){
                m_cb();
            }
        }

        timestamp expiration() const
        {
            return m_expiration;
        }

        bool repeat() const
        {
            return m_repeat;
        }

        void restart(timestamp when);


    private:

        const timer_callback_t m_cb;
        timestamp m_expiration;
        const double m_interval;
        const bool m_repeat;
    };
}

#endif //HANDSOME_TIMER_H
