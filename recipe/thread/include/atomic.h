//
// Created by gwe on 21-4-7.
//

#ifndef MISC_TEST_ATOMIC_H
#define MISC_TEST_ATOMIC_H

#include <stdint.h>

namespace handsome
{
    namespace detail{
        template<typename T>
        class atomic_integer
        {
        public:
            atomic_integer()
                : m_value(0)
            {

            }

            atomic_integer(const atomic_integer& rhs)
                : m_value(rhs.m_value)
            {

            }

            atomic_integer& operator=(const atomic_integer& rhs)
            {
                get_and_set(rhs.get());
                return *this;
            }

            T get() const
            {
                return __sync_val_compare_and_swap(const_cast<volatile T*>(&m_value), 0, 0);
            }

            T get_and_add(T x)
            {
                return __sync_fetch_and_add(&m_value, x);
            }

            T add_and_get(T x)
            {
                return get_and_add(x) + x;
            }

            T increment_and_get()
            {
                return add_and_get(1);
            }

            void add(T x)
            {
                get_and_add(x);
            }

            void increment()
            {
                increment_and_get();
            }

            void decrement()
            {
                get_and_add(-1);
            }

            T get_and_set(T new_value)
            {
                return __sync_lock_test_and_set(&m_value, new_value);
            }
        private:
            volatile T m_value;
        };
    }

    typedef detail::atomic_integer<int32_t> atomic_int32_t;
    typedef detail::atomic_integer<int64_t> atomic_int64_t;
}

#endif //MISC_TEST_ATOMIC_H
