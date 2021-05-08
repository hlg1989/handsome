//
// Created by gwe on 21-4-30.
//

#ifndef HANDSOME_BUFFER_H
#define HANDSOME_BUFFER_H

#include <algorithm>
#include <string>
#include <vector>

#include <assert.h>
#include <unistd.h>

namespace handsome{

    class buffer
    {
    public:

        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        buffer()
            : m_buffer(kCheapPrepend + kInitialSize)
            , m_read_index(kCheapPrepend)
            , m_write_index(kCheapPrepend)
        {

        }

        void swap(buffer& rhs)
        {
            m_buffer.swap(rhs.m_buffer);
            std::swap(m_read_index, rhs.m_read_index);
            std::swap(m_write_index, rhs.m_write_index);
        }

        size_t readable_bytes() const
        {
            return m_write_index - m_read_index;
        }

        size_t writable_bytes() const
        {
            return m_buffer.size() - m_write_index;
        }

        size_t prependable_bytes() const
        {
            return m_read_index;
        }

        const char* peek() const
        {
            return begin() + m_read_index;
        }

        void retrieve(size_t len)
        {
            assert(len <= readable_bytes());
            m_read_index += len;
        }

        void retrieve_until(const char* end)
        {
            assert(peek() <= end);
            assert(end <= begin_write());
            retrieve(end - peek());
        }

        void retrieve_all()
        {
            m_read_index = kCheapPrepend;
            m_write_index = kCheapPrepend;
        }

        std::string retrieve_as_string()
        {
            std::string str(peek(), readable_bytes());
            retrieve_all();
            return str;
        }

        void append(const std::string& str)
        {
            append(str.data(), str.size());
        }

        void append(const char* data, size_t len)
        {
            ensure_writable_bytes(len);
            std::copy(data, data + len, begin_write());
            has_written(len);
        }

        void append(const void* data, size_t len)
        {
            append(static_cast<const char*>(data), len);
        }

        void ensure_writable_bytes(size_t len)
        {
            if(writable_bytes() < len)
            {
                make_space(len);
            }

            assert(writable_bytes() > len);
        }

        char* begin_write()
        {
            return begin() + m_write_index;
        }

        void has_written(size_t len)
        {
            m_write_index += len;
        }

        void prepend(const void* data, size_t len)
        {
            assert(len <= prependable_bytes());
            m_read_index -= len;
            const char* d = static_cast<const char*>(data);
            std::copy(d, d + len, begin() + m_read_index);
        }

        void shrink(size_t reserve)
        {
            std::vector<char> buf(kCheapPrepend + readable_bytes() + reserve);
            std::copy(peek(), peek() + readable_bytes(), buf.begin() + kCheapPrepend);
            buf.swap(m_buffer);
        }

        ssize_t read_fd(int fd, int* save_errno);

    private:
        char* begin()
        {
            return m_buffer.data();
        }

        const char* begin() const
        {
            return m_buffer.data();
        }

        void make_space(size_t len)
        {
            if(writable_bytes() + prependable_bytes() < len + kCheapPrepend){
                m_buffer.resize(m_write_index + len);
            } else{
                assert(kCheapPrepend < m_read_index);
                size_t readable = readable_bytes();
                std::copy(begin() + m_read_index, begin() + m_write_index, begin() + kCheapPrepend);
                m_read_index = kCheapPrepend;
                m_write_index = m_read_index + readable;
                assert(readable == readable_bytes());
            }

        }

    private:
        std::vector<char> m_buffer;
        ssize_t m_read_index;
        ssize_t m_write_index;
    };
}

#endif //HANDSOME_BUFFER_H
