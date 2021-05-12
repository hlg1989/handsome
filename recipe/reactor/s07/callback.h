//
// Created by gwe on 21-4-30.
//

#ifndef HANDSOME_CALLBACK_H
#define HANDSOME_CALLBACK_H

#include "datetime/timestamp.h"

#include <functional>
#include <memory>


namespace handsome{

    class buffer;
    class tcp_connection;

    typedef std::function<void()> timer_callback_t;
    typedef std::function<void(const std::shared_ptr<tcp_connection>&)> connection_callback_t;
    typedef std::function<void(const std::shared_ptr<tcp_connection>&, buffer* buf, timestamp)> message_callback_t;
    typedef std::function<void(const std::shared_ptr<tcp_connection>&)> close_callback_t;

}

#endif //HANDSOME_CALLBACK_H