//
// Created by gwe on 21-4-30.
//

#ifndef HANDSOME_INET_ADDRESS_H
#define HANDSOME_INET_ADDRESS_H

#include <string>
#include <netinet/in.h>

namespace handsome{

    class inet_address
    {
    public:

        explicit inet_address(uint16_t port);

        inet_address(const std::string& ip, uint16_t port);

        inet_address(const struct sockaddr_in& addr)
            : m_addr(addr)
        {

        }

        std::string to_host_port() const;

        const struct sockaddr_in& get_sock_addr_inet() const
        {
            return m_addr;
        }

        void set_sock_addr_inet(const struct sockaddr_in& addr)
        {
            m_addr = addr;
        }

    private:

        struct sockaddr_in m_addr;
    };
}

#endif //HANDSOME_INET_ADDRESS_H
