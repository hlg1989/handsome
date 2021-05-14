//
// Created by gwe on 21-5-13.
//

#ifndef HANDSOME_SOCKET_OPS_H
#define HANDSOME_SOCKET_OPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace handsome{

    namespace socket_ops{

        inline uint64_t host_to_network64(uint64_t host64)
        {
            return htobe64(host64);
        }

        inline uint32_t host_to_network32(uint32_t host32)
        {
            return htobe32(host32);
        }

        inline uint16_t host_to_network16(uint16_t host16)
        {
            return htobe16(host16);
        }
        
        
        inline uint64_t network_to_host64(uint64_t net64)
        {
            return be64toh(net64);
        }

        inline uint32_t network_to_host32(uint32_t net32)
        {
            return be32toh(net32);
        }

        inline uint16_t network_to_host16(uint16_t net16)
        {
            return be16toh(net16);
        }


        int create_non_blocking_or_die();

        void bind_or_die(int sockfd, const struct sockaddr_in& addr);

        void listen_or_die(int sockfd);

        int accept(int sockfd, struct sockaddr_in* addr);

        void close(int sockfd);

        void shutdown_write(int sockfd);

        void to_host_port(char* buf, size_t size, const struct sockaddr_in& addr);

        void from_host_port(const char* ip, uint16_t port, struct sockaddr_in* addr);

        struct sockaddr_in get_sockaddr_in(int sockfd);

        int get_socket_error(int sockfd);
    }
}

#endif //HANDSOME_SOCKET_OPS_H
