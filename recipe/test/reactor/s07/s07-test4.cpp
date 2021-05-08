//
// Created by gwe on 21-4-23.
//

#include "event_loop.h"
#include "thread/thread.h"
#include "datetime/timestamp.h"
#include <functional>
#include <stdio.h>
#include <unistd.h>

int cnt = 0;
handsome::event_loop* g_loop;

void print_tid()
{
    printf("pid = %d, tid = %d\n", getpid(), handsome::current_thread::tid());
    printf("now %s \n", handsome::timestamp::now().to_string().c_str());
}

void print(const char* msg)
{
    printf("msg %s %s\n", handsome::timestamp::now().to_string().c_str(), msg);
    if(++cnt == 20){
        g_loop->quit();
    }
}

int main()
{
    print_tid();
    handsome::event_loop loop;
    g_loop = &loop;

    printf("main");

    loop.run_after(1, std::bind(print, "once1"));
    loop.run_after(1.5, std::bind(print, "once1.5"));
    loop.run_after(2.5, std::bind(print, "once2.5"));
    loop.run_after(3.5, std::bind(print, "once3.5"));

    loop.run_every(2, std::bind(print, "every2"));
    loop.run_every(3, std::bind(print, "every3"));

    loop.loop();
    printf("main loop exits");

    return 0;

}