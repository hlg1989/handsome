//
// Created by gwe on 21-4-8.
//

#ifndef MISC_TEST_THREAD_H
#define MISC_TEST_THREAD_H

#include "atomic.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <pthread.h>

namespace handsome{
    class thread : private noncopyable
    {
    public:
        typedef std::function<void()> thread_func;

        thread(thread_func func, const std::string& name = std::string());
        ~thread();

        void start();
        void join();

        bool started() const { return m_started; }
        pid_t tid() const { return *m_tid; }
        const std::string& name() const { return m_name; }

        static int created_nums() { return m_created_nums.get(); }

    private:
        bool m_started;
        bool m_joined;
        pthread_t m_pthread_id;
        std::shared_ptr<pid_t> m_tid;
        thread_func m_func;
        std::string m_name;

        static atomic_int32_t m_created_nums;
    };

    namespace current_thread{
        pid_t tid();
        const char* name();
        bool is_main_thread();
    }
}
#endif //MISC_TEST_THREAD_H
