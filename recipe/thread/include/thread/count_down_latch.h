//
// Created by gwe on 21-4-12.
//

#ifndef MISC_TEST_COUNT_DOWN_LATCH_H
#define MISC_TEST_COUNT_DOWN_LATCH_H

#include "mutex.h"
#include "condition.h"
#include "noncopyable.h"

namespace handsome
{
    class count_down_latch : private noncopyable
    {
    public:
        explicit count_down_latch(int count)
            : m_mtx()
            , m_cond(m_mtx)
            , m_count(count)
        {

        }

        void wait()
        {
            mutex_lock_guard lock(m_mtx);
            while(m_count > 0){
                m_cond.wait();
            }
        }

        void count_down()
        {
            mutex_lock_guard lock(m_mtx);
            --m_count;
            if(m_count == 0){
                m_cond.notify_all();
            }
        }

        int get_count() const
        {
            mutex_lock_guard lock(m_mtx);
            return m_count;
        }
    private:
        mutex m_mtx;
        condition m_cond;
        int m_count;
    };
}

#endif //MISC_TEST_COUNT_DOWN_LATCH_H
