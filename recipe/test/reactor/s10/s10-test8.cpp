//
// Created by gwe on 21-5-18.
//

#include "tcp_server.h"
#include "tcp_connection.h"
#include "event_loop.h"
#include "inet_address.h"

#include <stdio.h>

void on_connection(const std::shared_ptr<handsome::tcp_connection>& conn)
{
    if(conn->connected()){
        printf("on_connection(): tid = %d new connection [%s] from [%s]\n",
                handsome::current_thread::tid(),
                conn->name().c_str(),
                conn->peer_address().to_host_port().c_str());
    }else{
        printf("on_connection(): tid = %d connection [%s] is down\n",
                handsome::current_thread::tid(),
                conn->name().c_str());
    }
}

void on_message(const std::shared_ptr<handsome::tcp_connection>& conn,
                handsome::buffer* buf,
                handsome::timestamp receive_time)
{

    printf("on_message(): tid = %d received %zd bytes from connection [%s] at %s\n",
            handsome::current_thread::tid(),
            buf->readable_bytes(),
            conn->name().c_str(),
            receive_time.to_formatted_string().c_str());

    printf("on_message(): [%s]\n", buf->retrieve_as_string().c_str());

}

int main(int argc, char* argv[])
{
    printf("main(): pid = %d\n", getpid());

    handsome::inet_address listen_addr(9988);
    handsome::event_loop loop;

    handsome::tcp_server server(&loop, listen_addr);
    server.set_connection_callback(on_connection);
    server.set_message_callback(on_message);

    if(argc > 1){
        server.set_thread_num(atoi(argv[1]));
    }

    server.start();

    loop.loop();

    return 0;
}