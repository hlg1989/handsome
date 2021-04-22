//
// Created by gwe on 21-4-8.
//

#ifndef MISC_TEST_MUTEX_H
#define MISC_TEST_MUTEX_H

#include "thread.h"
#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>

namespace handsome{
    class mutex : private noncopyable
    {
    public:
        mutex()
            : m_holder(0)
        {
            pthread_mutex_init(&m_mutex, nullptr);
        }

        ~mutex()
        {
            assert(m_holder == 0);
            pthread_mutex_destroy(&m_mutex);
        }

        bool is_locked_by_this_thread()
        {
            return m_holder = current_thread::tid();
        }

        void assert_locked()
        {
            assert(is_locked_by_this_thread());
        }

        void lock()
        {
            pthread_mutex_lock(&m_mutex);
            m_holder = current_thread::tid();
        }

        void unlock()
        {
            m_holder = 0;
            pthread_mutex_unlock(&m_mutex);
        }

        pthread_mutex_t* get_pthread_mutex()
        {
            return &m_mutex;
        }
    private:
        pthread_mutex_t m_mutex;
        pid_t m_holder;
    };

    class mutex_lock_guard : private noncopyable
    {
    public:
        explicit mutex_lock_guard(mutex& mtx)
            : m_mtx(mtx)
        {
            m_mtx.lock();
        }

        ~mutex_lock_guard()
        {
            m_mtx.unlock();
        }

    private:
        mutex& m_mtx;
    };
}

#define mutex_lock_gurad(x) error "Missing object name"

#endif //MISC_TEST_MUTEX_H
