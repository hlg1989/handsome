//
// Created by gwe on 21-4-15.
//

#ifndef MISC_TEST_LOGGING_STREAM_H
#define MISC_TEST_LOGGING_STREAM_H

#include <assert.h>
#include <string.h>
#include <string>
#include "thread/include/noncopyable.h"

namespace handsome{
    namespace detail{
        const int kSmallBufferSize = 4000;
        const int kLargeBufferSize = 4000 * 1000;

        template <int SIZE>
        class fixed_buffer : private noncopyable
        {
        public:
            fixed_buffer()
                : m_cur(m_data)
            {
                set_cookie(cookie_start);
            }

            ~fixed_buffer()
            {
                set_cookie(cookie_end);
            }

            void append(const char* buf, int len)
            {
                if(avail() > len){
                    memcpy(m_cur, buf, len);
                    m_cur += len;
                }
            }

            const char* data() const
            {
                return m_data;
            }

            int length() const
            {
                return m_cur - m_data;
            }

            char* current()
            {
                return m_cur;
            }

            int avail() const
            {
                return static_cast<int>(end() - m_cur);
            }

            void add(size_t len)
            {
                m_cur += len;
            }

            void reset()
            {
                m_cur = m_data;
            }

            void bzero()
            {
                ::bzero(m_data, sizeof(m_data));
            }

            const char* debug_string();

            void set_cookie(void (*cookie)())
            {
                cookie_func = cookie;
            }

            std::string as_string() const
            {
                return std::string(m_data, length());
            }

        private:
            const char* end() const
            {
                return m_data + sizeof(m_data);
            }

            static void cookie_start();
            static void cookie_end();

            void (*cookie_func)();
        private:
            char m_data[SIZE];
            char* m_cur;
        };
    }

    class T
    {
    public:
        T(const char* str, int len)
            : m_str(str)
            , m_len(len)
        {
            assert(strlen(str) == m_len);
        }

        const char* m_str;
        const std::size_t m_len;
    };


    class logging_stream : private noncopyable
    {
    private:
        typedef logging_stream self;

    public:
        typedef detail::fixed_buffer<detail::kSmallBufferSize> small_buffer;
    public:
        self& operator<<(bool v)
        {
            m_buffer.append(v ? "1" : "0", 1);
            return *this;
        }

        self& operator<<(short);
        self& operator<<(unsigned short);
        self& operator<<(int);
        self& operator<<(unsigned int);
        self& operator<<(long);
        self& operator<<(unsigned long);
        self& operator<<(long long);
        self& operator<<(unsigned long long);
        self& operator<<(double v);

        self& operator<<(const void*);

        self& operator<<(float v)
        {
            *this << static_cast<double>(v);
            return *this;
        }

        self& operator<<(char v)
        {
            m_buffer.append(&v, 1);
            return *this;
        }

        self& operator<<(const char* v)
        {
            m_buffer.append(v, strlen(v));
            return *this;
        }

        self& operator<<(const T&v)
        {
            m_buffer.append(v.m_str, v.m_len);
            return *this;
        }

        self& operator<<(const std::string& v)
        {
            m_buffer.append(v.c_str(), v.size());
            return *this;
        }

        void append(const char* data, int len)
        {
            m_buffer.append(data, len);
        }

        const small_buffer& buffer() const
        {
            return m_buffer;
        }

        void reset_buffer()
        {
            m_buffer.reset();
        }

    private:
        void static_check();

        template <typename T>
        void format_integer(T);

    private:
        small_buffer m_buffer;

        static const int kMaxNumericSize = 32;

    };

    class format
    {
    public:
        template<typename T>
            format(const char* fmt, T val);

        const char* data() const
        {
            return m_buf;
        }

        int length() const
        {
            return m_len;
        }

    private:
        char m_buf[32];
        int m_len;
    };

    inline logging_stream& operator<<(logging_stream& s, const format& fmt)
    {
        s.append(fmt.data(), fmt.length());
        return s;
    }
}

#endif //MISC_TEST_LOGGING_STREAM_H
