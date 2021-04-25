//
// Created by gwe on 21-4-22.
//

#include "event_loop.h"
#include "thread/thread.h"
#include <stdio.h>
#include <unistd.h>

void thread_func()
{
    printf("thread_func(): pid = %d, tid = %d\n", getpid(), handsome::current_thread::tid());

    handsome::event_loop loop;
    loop.loop();
}

int main(int argc, char* argv[])
{
    printf("main() pid = %d, tid = %d\n", getpid(), handsome::current_thread::tid());

    handsome::event_loop loop;

    handsome::thread thread(thread_func);
    thread.start();

    loop.loop();

    pthread_exit(nullptr);

    return 0;
}