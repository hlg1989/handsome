//
// Created by gwe on 21-4-8.
//

#include "thread/thread.h"
#include <unistd.h>

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <assert.h>

namespace handsome{
    namespace current_thread{
        __thread const char* thread_name = "unknown";
    }
}

namespace {
    __thread pid_t cached_tid = 0;

#if !__GLIBC_PREREQ(2, 30)
    pid_t gettid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }
#endif

    void after_fork()
    {
        cached_tid = gettid();
        handsome::current_thread::thread_name = "main";
    }

    class thread_name_initializer{
    public:
        thread_name_initializer()
        {
            handsome::current_thread::thread_name = "main";
            pthread_atfork(nullptr, nullptr, &after_fork);
        }
    };

    thread_name_initializer init;

    struct thread_data
    {
        typedef handsome::thread::thread_func thread_func;

        thread_data(thread_func func, const std::string& name, const std::shared_ptr<pid_t>& tid)
            : m_func(func)
            , m_name(name)
            , m_weak_tid(tid)
        {

        }

        void run_in_thread()
        {
            pid_t tid = handsome::current_thread::tid();
            std::shared_ptr<pid_t> ptid = m_weak_tid.lock();
            if(ptid){
                *ptid = tid;
                ptid.reset();
            }

            handsome::current_thread::thread_name = m_name.empty() ? "handsome_thread" : m_name.c_str();
            ::prctl(PR_SET_NAME, handsome::current_thread::thread_name);
            m_func();
            handsome::current_thread::thread_name = "finished";
        }

        thread_func m_func;
        std::string m_name;
        std::weak_ptr<pid_t> m_weak_tid;
    };

    void* start_thread(void* obj)
    {
        thread_data* data = static_cast<thread_data*>(obj);

        data->run_in_thread();

        delete data;

        return nullptr;
    }
}

namespace handsome {

    pid_t current_thread::tid()
    {
        if (cached_tid == 0) {
            cached_tid = gettid();
        }

        return cached_tid;
    }

    const char *current_thread::name()
    {
        return thread_name;
    }

    bool current_thread::is_main_thread()
    {
        return tid() == ::getpid();
    }

    atomic_int32_t thread::m_created_nums;

    thread::thread(thread_func func, const std::string &name)
        : m_started(false)
        , m_joined(false)
        , m_pthread_id(0)
        , m_tid(new pid_t(0))
        , m_func(func)
        , m_name(name)
    {
        m_created_nums.increment();
    }

    thread::~thread()
    {
        if (m_started && !m_joined) {
            pthread_detach(m_pthread_id);
        }
    }

    void thread::start()
    {
        assert(!m_started);
        m_started = true;

        thread_data* data = new thread_data(m_func, m_name, m_tid);
        if(pthread_create(&m_pthread_id, NULL, &start_thread, data)){
            m_started = false;
            delete data;
            abort();
        }
    }

    void thread::join()
    {
        assert(m_started);
        assert(!m_joined);
        m_joined = true;
        pthread_join(m_pthread_id, nullptr);
    }

}