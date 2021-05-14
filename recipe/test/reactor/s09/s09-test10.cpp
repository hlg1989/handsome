//
// Created by gwe on 21-5-14.
//


#include "tcp_server.h"
#include "event_loop.h"
#include "inet_address.h"

#include <stdio.h>

std::string message1;
std::string message2;
int sleep_seconds = 5;

void on_connection(const std::shared_ptr<handsome::tcp_connection>& conn)
{
    if(conn->connected()){
        printf("on_connection() : new connection [%s] from %s\n",
                conn->name().c_str(), conn->peer_address().to_host_port().c_str());

        if(sleep_seconds > 0){
            ::sleep(sleep_seconds);
        }

        conn->send(message1);
        conn->send(message2);
        conn->shutdown();
    }else{
        printf("on_connection() connection [%s] is down\n", conn->name().c_str());
    }
}

void on_message(const std::shared_ptr<handsome::tcp_connection>& conn, handsome::buffer* buf, handsome::timestamp receive_time)
{
    printf("on_message() receive %zd bytes from connection [%s] at %s\n", buf->readable_bytes(), conn->name().c_str(), receive_time.to_formatted_string().c_str());
    buf->retrieve_all();
}

int main(int argc, char* argv[])
{
    printf("main(): pid = %d\n", getpid());

    int len1 = 100;

    int len2 = 200;

    if(argc > 2){
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    if(argc > 3){
        sleep_seconds = atoi(argv[3]);
    }

    message1.resize(len1);
    message2.resize(len2);

    std::fill(message1.begin(), message1.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

    handsome::inet_address listen_addr(9988);
    handsome::event_loop loop;

    handsome::tcp_server server(&loop, listen_addr);
    server.set_connection_callback(on_connection);
    server.set_message_callback(on_message);

    server.start();

    loop.loop();

    return 0;
}

