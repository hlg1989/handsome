//
// Created by gwe on 21-4-12.
//

#include "thread/thread_pool.h"
#include "thread/exception.h"

#include <functional>
#include <assert.h>
#include <stdio.h>
#include <algorithm>

namespace handsome{

    thread_pool::thread_pool(const std::string &name)
        : m_mtx()
        , m_cond(m_mtx)
        , m_name(name)
        , m_running(false)
        , m_threads()
        , m_queue()
    {

    }

    thread_pool::~thread_pool()
    {
        if(m_running){
            stop();
        }
    }

    void thread_pool::start(int thread_nums)
    {
        assert(m_threads.empty());
        m_running = true;
        m_threads.reserve(thread_nums);

        for(int i = 0; i < thread_nums; ++i){
            char id[32];
            snprintf(id, sizeof(id), "%d", i);
            m_threads.push_back(std::make_shared<thread>(std::bind(&thread_pool::run_in_thread, this), m_name + id));
            m_threads[i]->start();
        }
    }

    void thread_pool::stop()
    {
        m_running = false;
        m_cond.notify_all();

        std::for_each(m_threads.begin(), m_threads.end(), std::bind(&thread::join, std::placeholders::_1));
    }

    void thread_pool::run(const Task &f)
    {
        if(m_threads.empty()){
            f();
        }else{
            mutex_lock_guard lock(m_mtx);
            m_queue.push_back(f);
            m_cond.notify();
        }
    }

    thread_pool::Task thread_pool::take()
    {
        mutex_lock_guard lock(m_mtx);
        while(m_queue.empty() && m_running){
            m_cond.wait();
        }

        Task task;
        if(!m_queue.empty()){
            task = m_queue.front();
            m_queue.pop_front();
        }

        return task;
    }

    void thread_pool::run_in_thread()
    {
        try{
            while(m_running){
                Task task(take());
                if(task){
                    task();
                }
            }
        }catch(const exception& e){
            fprintf(stderr, "exception caught in thread_pool : %s\n", m_name.c_str());
            fprintf(stderr, "reason: %s\n", e.what());
            fprintf(stderr, "stack trace: %s\n", e.stack_trace());
            abort();
        }catch (const std::exception& e){
            fprintf(stderr, "exception caught in thread_pool %s\n", m_name.c_str());
            fprintf(stderr, "reason: %s\n", e.what());
            abort();
        }catch (...){
            fprintf(stderr, "unkown exception caught in thread_pool %s\n", m_name.c_str());
            abort();
        }
    }
}
