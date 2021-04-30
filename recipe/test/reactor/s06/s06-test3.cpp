//
// Created by gwe on 21-4-22.
//

#include "channel.h"
#include "event_loop.h"

#include <stdio.h>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>

handsome::event_loop* g_loop;

void timeout()
{
    printf("timeout!\n");
    g_loop->quit();
}

int main(int argc, char* argv[])
{
    handsome::event_loop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    handsome::channel channel(&loop, timerfd);
    channel.set_read_callback(timeout);
    channel.enable_reading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);

    loop.loop();

    ::close(timerfd);

    return 0;
}