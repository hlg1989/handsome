//
// Created by gwe on 21-4-27.
//

#ifndef HANDSOME_CALLBACK_H
#define HANDSOME_CALLBACK_H

#include <functional>
#include <memory>

namespace handsome{

    class tcp_connection;

    typedef std::function<void()> timer_callback_t;

    typedef std::function<void(const std::shared_ptr<tcp_connection>&)> connection_callback_t;

    typedef std::function<void(const std::shared_ptr<tcp_connection>&, const char*, ssize_t)> message_callback_t;
}

#endif //HANDSOME_CALLBACK_H
