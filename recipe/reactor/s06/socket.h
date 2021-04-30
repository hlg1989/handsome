//
// Created by gwe on 21-4-29.
//

#ifndef HANDSOME_SOCKET_H
#define HANDSOME_SOCKET_H


#include "thread/noncopyable.h"

namespace handsome{

    class inet_address;

    class socket : private noncopyable
    {
    public:

        explicit socket(int sockfd)
            : m_sockfd(sockfd)
        {

        }

        ~socket();

        int fd() const
        {
            return m_sockfd;
        }

        void bind_address(const inet_address& local_addr);

        void listen();

        int accept(inet_address* peer_addr);

        void set_reuse_addr(bool on);

    private:

        const int m_sockfd;
    };
}

#endif //HANDSOME_SOCKET_H
