//
// Created by gwe on 21-4-16.
//

#include "logging/logging_file.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

namespace handsome{

    class logging_file::output_file : private noncopyable
    {
    public:

        explicit output_file(const std::string& filename)
            : m_fp(::fopen(filename.c_str(), "ae"))
            , m_written_bytes(0)
        {
            ::setbuffer(m_fp, m_buffer, sizeof(m_buffer));
        }

        ~output_file()
        {
            ::fclose(m_fp);
        }

        void append(const char* logline, const size_t len)
        {
            size_t n = write(logline, len);
            size_t remain = len - n;

            while(remain > 0){
                size_t x = write(logline + n, remain);
                if(x == 0){
                    int err = ferror(m_fp);
                    if(err){
                        char buf[128];
                        strerror_r(err, buf, sizeof(buf));
                        fprintf(stderr, "logging_file::output_file::append() failed: %s\n", buf);
                    }

                    break;
                }

                n += x;
                remain = len - n;
            }
        }

        void flush()
        {
            ::fflush(m_fp);
        }

        size_t written_bytes() const
        {
            return m_written_bytes;
        }

    private:
        size_t write(const char* logline, size_t len)
        {
#undef fwrite_unlocked
            return ::fwrite_unlocked(logline, 1, len, m_fp);
        }



    private:

        FILE* m_fp;
        char m_buffer[64 * 1024];
        size_t m_written_bytes;
    };



    logging_file::logging_file(const std::string &basename, size_t roll_size, bool thread_safe, int flush_interval)
        : m_basename(basename)
        , m_roll_size(roll_size)
        , m_flush_interval(flush_interval)
        , m_count(0)
        , m_mutex(thread_safe ? new mutex() : nullptr)
        , m_start_of_period(0)
        , m_last_roll(0)
        , m_last_flush(0)
    {
        assert(basename.find('/') == std::string::npos);
        roll_file();
    }

    logging_file::~logging_file()
    {

    }

    void logging_file::append(const char *logline, int len)
    {
        if(m_mutex){
            mutex_lock_guard lock(*m_mutex);
            append_unlocked(logline, len);
        }else{
            append_unlocked(logline, len);
        }
    }

    void logging_file::flush()
    {
        if(m_mutex){
            mutex_lock_guard lock(*m_mutex);
            m_file->flush();
        }else{
            m_file->flush();
        }
    }

    void logging_file::append_unlocked(const char *logline, int len)
    {
        m_file->append(logline, len);

        if(m_file->written_bytes() > m_roll_size){
            roll_file();
        }else{
            if(m_count > kCountThreshold){
                m_count = 0;
                time_t now = ::time(nullptr);
                time_t this_period = now / kRollPerSeconds * kRollPerSeconds;
                if(this_period != m_start_of_period){
                    roll_file();
                }else if(now - m_last_flush > m_flush_interval){
                    m_last_flush = now;
                    m_file->flush();
                }
            }else{
                ++m_count;
            }
        }
    }

    void logging_file::roll_file()
    {
        time_t now = 0;
        std::string filename = get_logging_filename(m_basename, &now);
        time_t start = now / kRollPerSeconds * kRollPerSeconds;

        if(now > m_last_roll){
            m_last_roll = now;
            m_last_flush = now;
            m_start_of_period = start;
            m_file.reset(new output_file(filename));
        }
    }

    std::string logging_file::get_logging_filename(const std::string &basename, time_t *now)
    {
        std::string filename;
        filename.reserve(basename.size() + 32);
        filename = basename;

        char timebuf[32];
        char pidbuf[32];
        struct tm tm;
        *now = time(nullptr);
        gmtime_r(now, &tm);
        strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);
        filename += timebuf;
        snprintf(pidbuf, sizeof(pidbuf), ".%d", ::getpid());
        filename += pidbuf;
        filename += ".log";

        return filename;
    }
}