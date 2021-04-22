//
// Created by gwe on 21-4-12.
//

#ifndef MISC_TEST_BLOCKING_QUEUE_H
#define MISC_TEST_BLOCKING_QUEUE_H

#include "mutex.h"
#include "condition.h"
#include "noncopyable.h"

#include <deque>
#include <assert.h>

namespace handsome{
    template<typename T>
    class blocking_queue : private noncopyable
    {
    public:
        blocking_queue()
            : m_mtx()
            , m_cond()
            , m_queue()
        {

        }

        void put(const T& x)
        {
            mutex_lock_guard lock(m_mtx);
            m_queue.push_back(x);
            m_cond.notify();
        }

        T take()
        {
            mutex_lock_guard lock(m_mtx);
            while(m_queue.empty()){
                m_cond.wait();
            }

            assert(!m_queue.empty());
            T front(m_queue.front());
            m_queue.pop_front();
            return front;
        }

        size_t size() const
        {
            mutex_lock_guard lock(m_mtx);
            return m_queue.size();
        }

    private:
        mutable mutex m_mtx;
        condition m_cond;
        std::deque<T> m_queue;
    };
}
#endif //MISC_TEST_BLOCKING_QUEUE_H
