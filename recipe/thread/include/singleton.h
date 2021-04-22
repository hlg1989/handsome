//
// Created by gwe on 21-4-9.
//

#ifndef MISC_TEST_SINGLETON_H
#define MISC_TEST_SINGLETON_H

#include "noncopyable.h"
#include <pthread.h>
#include <stdlib.h>

namespace handsome{

    template<typename T>
    class singleton : private noncopyable
    {
    public:
        static T& instance()
        {
            pthread_once(&m_ponce, &singleton::init);
            return *m_value;
        }

    private:
        singleton() = default;
        ~singleton() = default;

        static void init()
        {
            m_value = new T();
            ::atexit(destroy);
        }

        static void destory()
        {
            delete m_value;
        }

    private:
        static pthread_once_t m_ponce;
        static T* m_value;
    };

    template<typename T>
    pthread_once_t singleton<T>::m_ponce = PTHREAD_ONCE_INIT;

    template<typename T>
    T* singleton<T>::m_value = nullptr;
}

#endif //MISC_TEST_SINGLETON_H
