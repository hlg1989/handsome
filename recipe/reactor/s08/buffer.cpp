//
// Created by gwe on 21-5-11.
//

#include "buffer.h"
#include "socket_ops.h"
#include "logging/logger.h"

#include <errno.h>
#include <sys/uio.h>

namespace handsome{

    ssize_t buffer::read_fd(int fd, int *save_errno)
    {
        char extra_buf[65536];
        struct iovec vec[2];
        const size_t writable = writable_bytes();

        vec[0].iov_base = begin() + m_writer_index;
        vec[0].iov_len = writable;

        vec[1].iov_base = extra_buf;
        vec[1].iov_len = sizeof(extra_buf);

        const ssize_t n = readv(fd, vec, 2);

        if(n < 0){
            *save_errno = errno;
        }else if(implicit_cast<size_t>(n) <= writable){
            m_writer_index += n;
        }else{
            m_writer_index = m_buffer.size();
            append(extra_buf, n - writable);
        }

        return n;
    }
}