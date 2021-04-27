//
// Created by gwe on 21-4-25.
//

#include "event_loop.h"
#include "event_loop_thread.h"
#include <stdio.h>
#include <unistd.h>

void run_in_thread()
{
    printf("run_in_thread(): pid = %d, tid = %d\n", getpid(), handsome::current_thread::tid());
}

int main()
{
    printf("main(): pid = %d, tid = %d\n", getpid(), handsome::current_thread::tid());

    handsome::event_loop_thread loop_thread;
    handsome::event_loop* loop = loop_thread.start_loop();

    loop->run_in_loop(run_in_thread);
    sleep(1);

    loop->run_after(2, run_in_thread);
    sleep(3);

    loop->quit();

    printf("exit main().\n");

    return 0;
}