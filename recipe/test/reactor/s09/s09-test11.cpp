//
// Created by gwe on 21-5-14.
//


#include "tcp_server.h"
#include "event_loop.h"
#include "inet_address.h"

#include <stdio.h>

std::string message;

void on_connection(const std::shared_ptr<handsome::tcp_connection>& conn)
{
    if(conn->connected()){
        printf("on_connection() : new connection [%s] from %s\n",
                conn->name().c_str(), conn->peer_address().to_host_port().c_str());

        conn->send(message);
    }else{
        printf("on_connection() connection [%s] is down\n", conn->name().c_str());
    }
}

void on_write_complete(const std::shared_ptr<handsome::tcp_connection>& conn)
{
    conn->send(message);
}

void on_message(const std::shared_ptr<handsome::tcp_connection>& conn, handsome::buffer* buf, handsome::timestamp receive_time)
{
    printf("on_message() receive %zd bytes from connection [%s] at %s\n", buf->readable_bytes(), conn->name().c_str(), receive_time.to_formatted_string().c_str());
    buf->retrieve_all();
}

int main(int argc, char* argv[])
{
    printf("main(): pid = %d\n", getpid());

    std::string line;

    for(int i = 33; i < 127; ++i){
        line.push_back(char(i));
    }

    line += line;

    for(size_t i = 0; i < 127 - 33; ++i){
        message += line.substr(i, 72) + "\n";
    }

    handsome::inet_address listen_addr(9988);
    handsome::event_loop loop;

    handsome::tcp_server server(&loop, listen_addr);
    server.set_connection_callback(on_connection);
    server.set_write_complete_callback(on_write_complete);
    server.set_message_callback(on_message);

    server.start();

    loop.loop();

    return 0;
}

