//
// Created by gwe on 21-4-27.
//

#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket_ops.h"

#include <stdio.h>
#include <unistd.h>

void new_connection(int sockfd, const handsome::inet_address& peer_addr)
{
    printf("new_connection() : accept a new connection from %s\n",
            peer_addr.to_host_port().c_str());

    ::write(sockfd, "how are you?\n", 13);

    handsome::socket_ops::close(sockfd);
}

int main()
{
    printf("main() : pid = %d\n", getpid());

    handsome::inet_address listen_addr(9988);
    handsome::event_loop loop;

    handsome::acceptor acceptor(&loop, listen_addr);
    acceptor.set_new_connection_callback(new_connection);
    acceptor.listen();

    loop.loop();
}