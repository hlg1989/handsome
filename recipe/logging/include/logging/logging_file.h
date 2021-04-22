//
// Created by gwe on 21-4-16.
//

#ifndef MISC_TEST_LOGGING_FILE_H
#define MISC_TEST_LOGGING_FILE_H

#include "thread/mutex.h"
#include "thread/noncopyable.h"

#include <string>
#include <memory>

namespace handsome{

    class logging_file : private noncopyable
    {
    public:
        logging_file(const std::string& basename, size_t roll_size, bool thread_safe = false, int flush_interval = 3);
        ~logging_file();

        void append(const char* logline, int len);
        void flush();

    private:
        void append_unlocked(const char* logline, int len);

        static std::string get_logging_filename(const std::string& basename, time_t* now);
        void roll_file();

    private:
        const std::string m_basename;
        const size_t m_roll_size;
        const int m_flush_interval;

        int m_count;

        std::unique_ptr<mutex> m_mutex;
        time_t m_start_of_period;
        time_t m_last_roll;
        time_t m_last_flush;

        class output_file;
        std::unique_ptr<output_file> m_file;

        static const int kCountThreshold = 1024;
        static const int kRollPerSeconds = 60 * 24 * 24;
    };
}

#endif //MISC_TEST_LOGGING_FILE_H
