//
// Created by gwe on 21-4-16.
//

#include "logging/logger.h"

#include "datetime/timestamp.h"
#include "thread/thread.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace handsome
{
    __thread char t_errnobuf[512];
    __thread char t_time[32];
    __thread time_t t_last_second;

    const char* strerror_tl(int saved_errno)
    {
        return strerror_r(saved_errno, t_errnobuf, sizeof(t_errnobuf));
    }

    logger::LOG_LEVEL init_log_level()
    {
        if(getenv("HANDSOME_LG_TRACE"))
            return logger::TRACE;
        else
            return logger::DEBUG;
    }

    logger::LOG_LEVEL g_log_level = init_log_level();

    const char* log_level_name[logger::NUM_LOG_LEVELS] =
        {
            "TRACE ",
            "DEBUG ",
            "INFO  ",
            "WARN  ",
            "ERROR ",
            "FATAL ",
        };

    void default_output(const char* msg, int len)
    {
        size_t n = fwrite(msg, 1, len, stdout);
    }

    void default_flush()
    {
        fflush(stdout);
    }

    logger::output_func g_output = default_output;
    logger::flush_func  g_flush = default_flush;

    logger::logger_impl::logger_impl(LOG_LEVEL level, int old_errno, const char *file, int line)
        : m_timestamp(timestamp::now())
        , m_stream()
        , m_level(level)
        , m_line(line)
        , m_fullname(file)
        , m_basename(nullptr)
    {
        const char* path_sep_pos = strrchr(m_fullname, '/');
        m_basename = (path_sep_pos != nullptr) ? path_sep_pos + 1 : m_fullname;

        format_time();

        format tid("%5d ", current_thread::tid());
        assert(tid.length() == 6);
        m_stream << T(tid.data(), 6);
        m_stream << T(log_level_name[level], 6);
        if(old_errno){
            m_stream << strerror_tl(old_errno) << " (errno = " << old_errno << ") ";
        }

    }

    void logger::logger_impl::format_time()
    {
        int64_t microseconds_since_epoch = m_timestamp.microseconds_since_epoch();
        time_t seconds = static_cast<time_t>(microseconds_since_epoch / timestamp::kMicroSecondsPerSecond);
        int microseconds = static_cast<int>(microseconds_since_epoch % timestamp::kMicroSecondsPerSecond);

        if(seconds != t_last_second){
            t_last_second = seconds;
            struct tm tm_time;
            gmtime_r(&seconds, &tm_time);

            int len = snprintf(t_time, sizeof(t_time), "%04d%02d%02d %02d:%02d:%02d",
                    tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                    tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

            assert(len == 17);
            (void)len;
        }

        format us(".%06dZ ",  microseconds);
        assert(us.length() == 9);
        m_stream << T(t_time, 17) << T(us.data(), 9);
    }

    void logger::logger_impl::finish()
    {
        m_stream << " - " << m_basename << ":" << m_line << '\n';
    }

    logger::logger(const char *file, int line)
        : m_impl(INFO, 0, file, line)
    {

    }

    logger::logger(const char *file, int line, LOG_LEVEL level, const char *func)
        : m_impl(level, 0, file, line)
    {
        m_impl.m_stream << func << " ";
    }

    logger::logger(const char *file, int line, LOG_LEVEL level)
        : m_impl(level, 0, file, line)
    {

    }

    logger::logger(const char *file, int line, bool to_abort)
        : m_impl(to_abort ? FATAL : ERROR, errno, file, line)
    {

    }

    logger::~logger()
    {
        m_impl.finish();
        const logging_stream::small_buffer& buf = stream().buffer();
        g_output(buf.data(), buf.length());
        if(m_impl.m_level == FATAL){
            g_flush();
            abort();
        }
    }

    logger::LOG_LEVEL logger::log_level()
    {
        return g_log_level;
    }

    void logger::set_log_level(LOG_LEVEL level)
    {
        g_log_level = level;
    }

    void logger::set_output(output_func func)
    {
        g_output = func;
    }

    void logger::set_flush(flush_func func)
    {
        g_flush = func;
    }
}