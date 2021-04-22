//
// Created by gwe on 21-4-9.
//

#ifndef MISC_TEST_THREAD_LOCAL_SINGLETON_H
#define MISC_TEST_THREAD_LOCAL_SINGLETON_H

#include "noncopyable.h"
#include <pthread.h>

namespace handsome{
    template <typename T>
    class thread_local_singleton : private noncopyable
    {
    public:
        static T& instance()
        {
            auto& item = init();
            return item;
        }

    private:
        thread_local_singleton() = delete;
        ~thread_local_singleton() = delete;

    private:
        static T& init()
        {
            auto& pkey = m_deleter.key();
            T* val = static_cast<T*>(pthread_getspecific(pkey));
            if(!val){
                T* new_val = new T();

                pthread_setspecific(pkey, new_val);
                val = new_val;
            }

            return *val;
        }

        static void destructor(void* obj)
        {
            delete obj;
        }

        class Deleter
        {
        public:
            Deleter()
            {
                pthread_key_create(&m_pkey, &thread_local_singleton::destructor);
            }

            ~Deleter()
            {
                pthread_key_delete(m_pkey);
            }


            pthread_key_t& key()
            {
                return m_pkey;
            }
        private:
            pthread_key_t m_pkey;

        };
    private:
        static Deleter m_deleter;
    };


    template<typename T>
    typename thread_local_singleton<T>::Deleter thread_local_singleton<T>::m_deleter;

}

#endif //MISC_TEST_THREAD_LOCAL_SINGLETON_H
