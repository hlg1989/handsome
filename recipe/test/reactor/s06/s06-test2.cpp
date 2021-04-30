//
// Created by gwe on 21-4-22.
//

#include "event_loop.h"
#include "thread/thread.h"

handsome::event_loop* g_loop;

void thread_func()
{
    g_loop->loop();
}

int main()
{
    handsome::event_loop loop;
    g_loop = &loop;

    handsome::thread t(thread_func);
    t.start();

    t.join();

    return 0;
}