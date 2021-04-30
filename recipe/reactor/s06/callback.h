//
// Created by gwe on 21-4-28.
//

#ifndef HANDSOME_CALLBACK_H
#define HANDSOME_CALLBACK_H

#include <memory>
#include <functional>

namespace handsome{

    class tcp_connection;

    typedef std::function<void()> timer_callback_t;

    typedef std::function<void(const std::shared_ptr<tcp_connection>&)> connection_callback_t;

    typedef std::function<void(const std::shared_ptr<tcp_connection>&, const char* data, ssize_t len)> message_callback_t;

    typedef std::function<void(const std::shared_ptr<tcp_connection>&)> close_callback_t;

}

#endif //HANDSOME_CALLBACK_H
