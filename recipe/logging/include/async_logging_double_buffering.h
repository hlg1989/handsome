//
// Created by gwe on 21-4-19.
//

#ifndef MISC_TEST_ASYNC_LOGGING_DOUBLE_BUFFERING_H
#define MISC_TEST_ASYNC_LOGGING_DOUBLE_BUFFERING_H

#include "logging_stream.h
#include "logging_file.h"
#include "thread/include/blocking_queue.h"
#include "thread/include/count_down_latch.h"
#include "thread/include/mutex.h"
#include "thread/include/thread.h"
#include "thread/include/noncopyable.h"

#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <string>

namespace handsome{

    class async_logging_double_buffering : private noncopyable
    {
    public:
        typedef handsome::detail::fixed_buffer<handsome::detail::kLargeBufferSize> buffer;

    public:
        async_logging_double_buffering(const std::string& basename, std::size_t roll_size, int flush_interval = 3)
            : m_flush_interval(flush_interval)
            , m_running(false)
            , m_basename(basename)
            , m_thread(std::bind(&async_logging_double_buffering::thread_func, this), "logging")
            , m_latch(1)
            , m_mtx()
            , m_cond(m_mtx)
            , m_current_buffer(new buffer())
            , m_next_buffer(new buffer())
            , m_buffers()
        {
            m_current_buffer->bzero();
            m_next_buffer->bzero();
            m_buffers.reserve(16);
        }

        ~async_logging_double_buffering()
        {
            if(m_running){
                stop();
            }
        }

        void append(const char* log_line, int len)
        {
            mutex_lock_guard lock(m_mtx);
            if(m_current_buffer->avail() > len){
                m_current_buffer->append(log_line, len);
            }else{
                m_buffers.push_back(std::move(m_current_buffer));

                if(m_next_buffer){
                    m_current_buffer = std::move(m_next_buffer);
                } else{
                    m_current_buffer.reset(new buffer());
                }
                m_current_buffer->append(log_line, len);

                m_cond.notify();
            }
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
            m_cond.notify();
            m_thread.stop();
        }

    private:

        void thread_func()
        {
            assert(m_running == true);
            m_latch.count_down();
            logging_file output(m_basename, m_roll_size, false);
            std::unique_ptr<buffer> new_buffer1(new buffer());
            std::unique_ptr<buffer> new_buffer2(new buffer());
            new_buffer1->bzero();
            new_buffer2->bzero();
            std::vector<std::unique_ptr<buffer>> buffer_to_write;
            buffer_to_write.reserve(16);

            while(m_running){
                assert(new_buffer1 && new_buffer1->length() == 0);
                assert(new_buffer1 && new_buffer2->length() == 0);
                assert(buffer_to_write.empty());

                {
                    mutex_lock_guard lock(m_mtx);
                    if(m_buffers.empty()){
                        m_cond.wait_for(m_flush_interval * 1000);
                    }

                    m_buffers.push_back(std::move(m_current_buffer));
                    m_current_buffer = std::move(new_buffer1);
                    buffer_to_write.swap(m_buffers);
                    if(!m_next_buffer){
                        m_next_buffer = std::move(new_buffer2);
                    }
                }

                assert(!buffer_to_write.empty());

                if(buffer_to_write.size() > 25){
                    const char* drop_message = "Dropped log messages\n";
                    fprintf(stderr, "%s", drop_message);
                    output.append(drop_message, strlen(drop_message));
                    buffer_to_write.erase(buffer_to_write.begin(), buffer_to_write.end() - 2);
                }

                for(std::size_t i = 0; i < buffer_to_write.size(); ++i){
                    output.append(buffer_to_write[i]->data(), buffer_to_write[i]->length());
                }

                if(buffer_to_write.size() > 2){
                    buffer_to_write.resize(2);
                }

                if(!new_buffer1){
                    assert(!buffer_to_write.empty());
                    new_buffer1 = std::move(buffer_to_write.pop_back());
                    new_buffer1->reset();
                }

                if(!new_buffer2){
                    assert(!buffer_to_write.empty());
                    new_buffer2 = std::move(buffer_to_write.pop_back());
                    new_buffer2->reset();
                }

                buffer_to_write.clear();
                output.flush();
            }

            output.flush();
        }

    private:
        const int m_flush_interval;
        std::atomic_bool m_running;
        std::string m_basename;
        std::size_t m_roll_size;
        thread m_thread;
        count_down_latch m_latch;
        mutex m_mtx;
        condition m_cond;
        std::unique_ptr<buffer> m_current_buffer;
        std::unique_ptr<buffer> m_next_buffer;
        std::vector<std::unique_ptr<buffer>> m_buffers;

    };
}

#endif //MISC_TEST_ASYNC_LOGGING_DOUBLE_BUFFERING_H
