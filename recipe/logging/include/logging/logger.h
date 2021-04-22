//
// Created by gwe on 21-4-16.
//

#ifndef MISC_TEST_LOGGER_H
#define MISC_TEST_LOGGER_H

#include "logging_stream.h"
#include "datetime/timestamp.h"
#include <memory>

namespace handsome
{
    class logger
    {
    public:
        enum LOG_LEVEL
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS
        };

        logger(const char* file, int line);
        logger(const char* file, int line, LOG_LEVEL level);
        logger(const char* file, int line, LOG_LEVEL level, const char* func);
        logger(const char* file, int line, bool to_abort);

        ~logger();

        logging_stream& stream()
        {
            return m_impl.m_stream;
        }

        static LOG_LEVEL log_level();
        static void set_log_level(LOG_LEVEL level);

        typedef void (*output_func)(const char* msg, int len);
        typedef void (*flush_func)();

        static void set_output(output_func);
        static void set_flush(flush_func);

    private:
        class logger_impl
        {
        public:
            typedef logger::LOG_LEVEL LOG_LEVEL;

            logger_impl(LOG_LEVEL level, int old_errno, const char* file, int line);
            void format_time();
            void finish();

            timestamp m_timestamp;
            logging_stream m_stream;
            LOG_LEVEL m_level;
            int m_line;
            const char* m_fullname;
            const char* m_basename;
        };

        logger_impl m_impl;
    };


#define LOG_TRACE   if(handsome::logger::log_level() <= handsome::logger::TRACE)    \
    handsome::logger(__FILE__, __LINE__, handsome::logger::TRACE, __FUNCTION__).stream()

#define LOG_DEBUG   if(handsome::logger::log_level() <= handsome::logger::DEBUG)    \
    handsome::logger(__FILE__, __LINE__, handsome::logger::DEBUG, __FUNCTION__).stream()

#define LOG_INFO   if(handsome::logger::log_level() <= handsome::logger::INFO)     \
    handsome::logger(__FILE__, __LINE__, handsome::logger::INFO, __FUNCTION__).stream()

#define LOG_WARN    handsome::logger(__FILE__, __LINE__, handsome::logger::WARN).stream()
#define LOG_ERROR   handsome::logger(__FILE__, __LINE__, handsome::logger::ERROR).stream()
#define LOG_FATAL   handsome::logger(__FILE__, __LINE__, handsome::logger::FATAL).stream()
#define LOG_SYSERR  handsome::logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL handsome::logger(__FILE__, __LINE__, true).stream()


    const char* strerror_tl(int saved_errno);

#define CHECK_NOT_NULL(val)     \
    handsome::check_not_null(__FILE__, __LINE__, "'"#val"' Must be non NULL", (val))

    template <typename T>
    T* check_not_null(const char* file, int line, const char* names, T* ptr)
    {
        if(!ptr){
            logger(file, line, logger::FATAL).stream() << names;
        }

        return ptr;
    }

    template <typename To, typename From>
    inline To implicit_cast(From const& f)
    {
        return f;
    };
}

#endif //MISC_TEST_LOGGER_H
