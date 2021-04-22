//
// Created by gwe on 21-4-19.
//

#ifndef MISC_TEST_ASYNC_LOGGING_QUEUE_H
#define MISC_TEST_ASYNC_LOGGING_QUEUE_H

#include "logging_file.h"
#include "thread/include/blocking_queue.h"
#include "thread/include/count_down_latch.h"
#include "thread/include/thread.h"
#include "thread/include/noncopyable.h"

#include <string>
#include <functional>
#include <memory>
#include <atomic>

namespace handsome{

    template<typename MESSAGE, template <typename >class QUEUE>
    class async_logging_t : private noncopyable
    {
        async_logging_t(const std::string& basename, std::size_t roll_size)
            : m_running(false)
            , m_basename(basename)
            , m_roll_size(roll_size)
            , m_thread(std::bind(&async_logging_t::thread_func, this), "logging")
            , m_latch(1)
        {

        }

        ~async_logging_t()
        {
            if(m_running){
                stop();
            }
        }

        void append(const char* log_line, int len)
        {
            m_queue.put(MESSAGE(log_line, len));
        }

        void start()
        {
            m_running = true;
            m_thread.start();
            m_latch.wait();
        }

        void stop()
        {
            m_running = false;
            m_queue.put(MESSAGE());
            m_thread.join();
        }

    private:
        void thread_func()
        {
            assert(m_running == true);
            m_latch.count_down();
            logging_file output(m_basename, m_roll_size, false);
            time_t last_flush = time(nullptr);

            while(true){
                MESSAGE msg(m_queue.take());
                if(msg.empty()){
                    assert(m_running == false);
                    break;
                }

                output.append(msg.data(), msg.length);
            }

            output.flush();
        }

    private:

        std::atomic_bool m_running;
        std::string m_basename;
        std::size_t m_roll_size;
        thread m_thread;
        count_down_latch m_latch;
        QUEUE<MESSAGE> m_queue;
    };

    struct log_message
    {
        log_message(const char* msg, int len)
            : m_length(len)
        {
            assert(m_length <= sizeof(m_data));
            ::memcpy(m_data, msg, len);
        }

        log_message()
            : m_length(0)
        {

        }

        log_message(const log_message& rhs)
            : m_length(rhs.m_length)
        {
            assert(m_length <= sizeof(m_data));
            ::memcpy(m_data, rhs.m_data, m_length);
        }

        log_message& operator=(const log_message& rhs)
        {
            m_length = rhs.m_length;
            assert(m_length <= sizeof(m_data));
            ::memcpy(m_data, rhs.m_data, m_length);
        }

        const char* data() const
        {
            return m_data;
        }

        int length() const
        {
            return m_length;
        }

        bool empty() const
        {
            return m_length == 0;
        }

        char m_data[4000];
        std::size_t m_length;
    };

    typedef async_logging_t<std::string, blocking_queue> async_string_logging_queue;
    typedef async_logging_t<log_message, blocking_queue> aysnc_message_logging_queue;
}

#endif //MISC_TEST_ASYNC_LOGGING_QUEUE_H
