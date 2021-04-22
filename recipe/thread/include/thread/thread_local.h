//
// Created by gwe on 21-4-9.
//

#ifndef MISC_TEST_THREAD_LOCAL_H
#define MISC_TEST_THREAD_LOCAL_H

#include "noncopyable.h"
#include <pthread>

namespace handsome
{
    template<typename T>
    class thread_local_variable : private noncopyable
    {
    public:
        thread_local_variable()
        {
            pthead_key_create(&m_pkey, &thread_local_variable::destroctor);
        }

        ~thread_local_variable()
        {
            pthread_key_delete(m_pkey);
        }

        T& value()
        {
            T* val = static_cast<T*>(pthread_getspecific(m_pkey));
            if(!val){
                T* new_val = new T();
                pthread_setspecific(m_key, new_val);
                val = new_val;
            }

            return *val;
        }

    private:
        static void destructor(void* x)
        {
            T* obj = static_cast<T*>(x);
            delete obj;
        }
    private:
        pthread_key_t m_pkey;
    };
}
#endif //MISC_TEST_THREAD_LOCAL_H
