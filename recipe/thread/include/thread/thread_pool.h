//
// Created by gwe on 21-4-12.
//

#ifndef MISC_TEST_THREAD_POOL_H
#define MISC_TEST_THREAD_POOL_H

#include "condition.h"
#include "mutex.h"
#include "thread.h"
#include "noncopyable.h"

#include <functional>
#include <memory>
#include <deque>
#include <vector>
#include <atomic>

namespace handsome
{
    class thread_pool : private noncopyable
    {
    public:
        typedef std::function<void()> Task;

        explicit thread_pool(const std::string& name = "");
        ~thread_pool();

        void start(int thread_nums);
        void stop();

        void run(const Task& f);

    private:
        void run_in_thread();
        Task take();

    private:
        mutex m_mtx;
        condition m_cond;
        std::string m_name;
        std::vector<std::shared_ptr<thread>> m_threads;
        std::deque<Task> m_queue;
        std::atomic_bool m_running;
    };
}
#endif //MISC_TEST_THREAD_POOL_H
