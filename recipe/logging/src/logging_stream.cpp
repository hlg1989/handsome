//
// Created by gwe on 21-4-15.
//

#include "logging_stream.h"
#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

namespace handsome{
    namespace detail{
        const char digits[] = "9876543210123456789";
        const char* zero = digits + 9;
        static_assert(sizeof(digits) == 20);

        const char digits_hex[] = "0123456789abcdef";
        static_assert(sizeof(digits_hex) == 17);

        template<typename T>
        size_t convert(char buf[], T value)
        {
            T i = value;
            char* p = buf;

            do{
                int lsd = i % 10;
                i /= 10;
                *p++ = zero[lsd];
            }while(i != 0);

            if(value < 0){
                *p++ = '-';
            }

            *p = '\0';

            std::reverse(buf, p);

            return p - buf;
        }

        size_t convert_hex(char buf[], uintptr_t value)
        {
            uintptr_t i = value;
            char* p = buf;

            do{
                int lsd = i % 16;
                i /= 16;
                *p++ = digits_hex[lsd];
            }while(i != 0);

            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }


        template <int SIZE> const char* fixed_buffer<SIZE>::debug_string()
        {
            *m_cur = '\0';
            return m_data;
        }

        template <int SIZE> void fixed_buffer<SIZE>::cookie_start()
        {

        }

        template <int SIZE>void fixed_buffer<SIZE>::cookie_end()
        {

        }

    }


    template class detail::fixed_buffer<detail::kSmallBufferSize>;
    template class detail::fixed_buffer<detail::kLargeBufferSize>;

    void logging_stream::static_check()
    {
        static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10);
        static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10);
        static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10);
        static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10);
    }

    template<typename T>
    void logging_stream::format_integer(T v)
    {
        if(m_buffer.avail() >= kMaxNumericSize){
            size_t len = detail::convert(m_buffer.current(), v);
            m_buffer.add(len);
        }
    }

    logging_stream& logging_stream::operator<<(short v)
    {
        *this << static_cast<int>(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(unsigned short v)
    {
        *this << static_cast<unsigned int >(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(int v)
    {
        format_integer(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(unsigned int v)
    {
        format_integer(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(long v)
    {
        format_integer(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(unsigned long v)
    {
        format_integer(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(long long v)
    {
        format_integer(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(unsigned long long v)
    {
        format_integer(v);
        return *this;
    }

    logging_stream& logging_stream::operator<<(const void* v)
    {
        uintptr_t p = reinterpret_cast<uintptr_t>(v);
        if(m_buffer.avail() >= kMaxNumericSize){
            char* buf = m_buffer.current();
            buf[0] = '0';
            buf[1] = 'x';
            std::size_t len = detail::convert_hex(buf + 2, p);
            m_buffer.add(len + 2);
        }

        return *this;
    }

    logging_stream& logging_stream::operator<<(double v)
    {
        if(m_buffer.avail() >= kMaxNumericSize){
            int len = snprintf(m_buffer.current(), kMaxNumericSize, "%.12g", v);
            m_buffer.add(len);
        }
    }

    template<typename T>
    format::format(const char *fmt, T val)
    {
        static_assert(std::is_arithmetic<T>::value == true);

        m_len = snprintf(m_buf, sizeof(m_buf), fmt, val);
        assert(static_cast<size_t>(m_len) < sizeof(m_buf));
    }

    template format::format(const char * fmt, short);
    template format::format(const char * fmt, unsigned short);
    template format::format(const char * fmt, int);
    template format::format(const char * fmt, unsigned int);
    template format::format(const char * fmt, long);
    template format::format(const char * fmt, unsigned long);
    template format::format(const char * fmt, long long);
    template format::format(const char * fmt, unsigned long long);
    template format::format(const char * fmt, float);
    template format::format(const char * fmt, double);


}