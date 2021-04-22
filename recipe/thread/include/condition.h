//
// Created by gwe on 21-4-8.
//

#ifndef MISC_TEST_CONDITION_H
#define MISC_TEST_CONDITION_H

#include "mutex.h"
#include "noncopyable.h"
#include <pthread.h>
#include <errno.h>
#include <time.h>

namespace handsome{
    class condition : private noncopyable
    {
    public:
        explicit condition(mutex& mtx)
            : m_mtx(mtx)
        {
            pthread_cond_init(&m_cond, nullptr);
        }

        ~condition()
        {
            pthread_cond_destroy(&m_cond);
        }

        void wait()
        {
            pthread_cond_wait(&m_cond, m_mtx.get_pthread_mutex());
        }

        bool wait_for(int milliseconds)
        {
            struct timespec abstime;
            clock_gettime(CLOCK_REALTIME, &abstime);
            long nanoseconds = milliseconds * 1000;
            const long nanoseconds_per_seconds = 1000 * 1000;
            abstime.tv_sec += nanoseconds / nanoseconds_per_seconds;
            abstime.tv_nsec += nanoseconds % nanoseconds_per_seconds;

            return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mtx.get_pthread_mutex(), &abstime);
        }

        void notify()
        {
            pthread_cond_signal(&m_cond);
        }

        void notify_all()
        {
            pthread_cond_broadcast(&m_cond);
        }

    private:
        mutex& m_mtx;
        pthread_cond_t m_cond;
    };
}

#endif //MISC_TEST_CONDITION_H
