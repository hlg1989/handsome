//
// Created by gwe on 21-5-21.
//

#include "connector.h"
#include "channel.h"
#include "event_loop.h"
#include "socket_ops.h"

#include "logging/logger.h"

#include <functional>

#include <errno.h>

namespace handsome{

    const int connector::kMaxRetryDelayMS;

    connector::connector(event_loop *loop, const inet_address &server_addr)
        : m_loop(loop)
        , m_server_addr(server_addr)
        , m_connect(false)
        , m_state(kDisconnected)
        , m_retry_delay_ms(kInitRetryDelayMS)
    {
        LOG_DEBUG << "ctor[" << this << "]";
    }

    connector::~connector()
    {
        LOG_DEBUG << "dtor[" << this << "]";
        m_loop->cancel(m_timer_id);
        assert(!m_channel);
    }

    void connector::start()
    {
        m_connect = true;
        m_loop->run_in_loop(std::bind(&connector::start_in_loop, this));
    }

    void connector::start_in_loop()
    {
        m_loop->assert_in_loop_thread();
        assert(m_state == kDisconnected);

        if(m_connect){
            connect();
        }else{
            LOG_DEBUG << "do not connect";
        }
    }

    void connector::connect()
    {
        int sockfd = socket_ops::create_non_blocking_or_die();
        int ret = socket_ops::connect(sockfd, m_server_addr.get_sockaddr_in());

        int saved_errno = (ret == 0) ? 0 : errno;

        switch (saved_errno){
            case 0:
            case EINPROGRESS:
            case EINTR:
            case EISCONN:
                connecting(sockfd);
                break;

            case EAGAIN:
            case EADDRINUSE:
            case EADDRNOTAVAIL:
            case ECONNREFUSED:
            case ENETUNREACH:
                retry(sockfd);
                break;

            case EACCES:
            case EPERM:
            case EAFNOSUPPORT:
            case EALREADY:
            case EBADF:
            case EFAULT:
            case ENOTSOCK:
                LOG_SYSERR << "conenct error in connector::start_in_loop " << saved_errno;
                socket_ops::close(sockfd);
                break;

            default:
                LOG_SYSERR << "unexpected error in connector::start_in_loop " << saved_errno;
                socket_ops::close(sockfd);
                break;
        }
    }

    void connector::restart()
    {
        m_loop->assert_in_loop_thread();
        set_state(kDisconnected);
        m_retry_delay_ms = kInitRetryDelayMS;
        m_connect = true;

        start_in_loop();
    }

    void connector::stop()
    {
        m_connect = false;
        m_loop->cancel(m_timer_id);
    }

    void connector::connecting(int sockfd)
    {
        set_state(kConnecting);
        assert(!m_channel);

        m_channel.reset(new channel(m_loop, sockfd));
        m_channel->set_write_callback(std::bind(&connector::handle_write, this));
        m_channel->set_error_callback(std::bind(&connector::handle_error, this));

        m_channel->enable_writing();
    }

    int connector::remove_and_reset_channel()
    {
        m_channel->disable_all();
        m_loop->remove_channel(m_channel.get());
        int sockfd = m_channel->fd();

        m_loop->queue_in_loop(std::bind(&connector::reset_channel, this));

        return sockfd;
    }

    void connector::reset_channel()
    {
        m_channel.reset();
    }

    void connector::handle_write()
    {
        LOG_TRACE << "connector::handle_write " << m_state.load();

        if(m_state == kConnecting){
            int sockfd = remove_and_reset_channel();
            int err = socket_ops::get_socket_error(sockfd);
            if(err){
                LOG_WARN << "connector::handle_write - SO_ERROR = "
                         << err << " " << strerror_tl(err);

                retry(sockfd);
            }else if(socket_ops::is_self_connect(sockfd)){
                LOG_WARN << "connector::handle_write - self connect";
                retry(sockfd);
            }else{
                set_state(kConnected);

                if(m_connect){
                    m_new_connection_cb(sockfd);
                }else{
                    socket_ops::close(sockfd);
                }
            }
        }else{
            assert(m_state == kDisconnected);
        }
    }

    void connector::handle_error()
    {
        LOG_ERROR << "connector::handle_error";
        assert(m_state == kConnecting);

        int sockfd = remove_and_reset_channel();
        int err = socket_ops::get_socket_error(sockfd);

        LOG_TRACE << "SO_ERROR = " << err << strerror_tl(err);
        retry(sockfd);
    }

    void connector::retry(int sockfd)
    {
        socket_ops::close(sockfd);
        set_state(kDisconnected);

        if(m_connect){
            LOG_INFO << "connector::retry - retry connecting to "
                     << m_server_addr.to_host_port() << " in "
                     << m_retry_delay_ms << " milliseconds";

            m_timer_id = m_loop->run_after(m_retry_delay_ms / 1000.0, std::bind(&connector::start_in_loop, this));

            m_retry_delay_ms = std::min(m_retry_delay_ms * 2, kMaxRetryDelayMS);
        }else{
            LOG_DEBUG << "do not connect";
        }
    }

}