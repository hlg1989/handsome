//
// Created by gwe on 21-5-21.
//

#include "connector.h"
#include "event_loop.h"

#include <stdio.h>

handsome::event_loop* g_loop;

void connection_callback(int sockfd)
{
    printf("connected.\n");
    g_loop->quit();
}

int main(int argc, char* argv[])
{
    handsome::event_loop loop;
    g_loop = &loop;

    handsome::inet_address addr("127.0.0.1", 9988);
    auto connector = std::make_shared<handsome::connector>(&loop, addr);
    connector->set_new_connection_callback(connection_callback);

    connector->start();

    loop.loop();
}