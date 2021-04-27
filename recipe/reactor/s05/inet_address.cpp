//
// Created by gwe on 21-4-27.
//

#include "inet_address.h"
#include "socket_ops.h"

#include <string.h>
#include <netinet/in.h>

namespace handsome{

    static const in_addr_t kInAddrAny = INADDR_ANY;

    static_assert(sizeof(inet_address) == sizeof(struct sockaddr_in));

    inet_address::inet_address(uint16_t port)
    {
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = socket_ops::host_to_network16(port);
        m_addr.sin_addr.s_addr = socket_ops::host_to_network32(kInAddrAny);
    }

    inet_address::inet_address(const std::string &ip, uint16_t port)
    {
        bzero(&m_addr, sizeof(m_addr));
        socket_ops::from_host_port(ip.c_str(), port, &m_addr);
    }

    std::string inet_address::to_host_port() const
    {
        char buf[32];
        socket_ops::to_host_port(buf, sizeof(buf), m_addr);
        return buf;
    }
}