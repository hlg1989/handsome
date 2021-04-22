//
// Created by gwe on 21-4-9.
//

#include "thread.h"
#include "thread_local_singleton.h"
#include "noncopyable.h"

#include <functional>
#include <unistd.h>

class test : noncopyable
{
public:
    test()
    {
        printf("tid = %d, constructing %p\n", handsome::current_thread::tid(), this);
    }

    ~test()
    {
        printf("tid = %d, destructing %p %s\n", handsome::current_thread::tid(), this, m_name);
    }

    const std::string& name() const
    {
        return m_name;
    }

    void set_name(const std::string& name)
    {
        m_name = name;
    }
private:
    std::string m_name = "unkown";
};

void thread_func(const char* name)
{
    printf("tid = %d, %p name = %s\n",
        handsome::current_thread::tid(),
        &handsome::thread_local_singleton<test>::instance(),
        handsome::thread_local_singleton<test>::instance().name().c_str());

    handsome::thread_local_singleton<test>::instance().set_name(name);

    sleep(1);

    printf("tid = %d, %p name = %s\n",
           handsome::current_thread::tid(),
           &handsome::thread_local_singleton<test>::instance(),
           handsome::thread_local_singleton<test>::instance().name().c_str());
}

int main()
{
    handsome::thread_local_singleton<test>::instance().set_name("hlg");
    handsome::thread t1(std::bind(thread_func, "thread1"));
    handsome::thread t2(std::bind(thread_func, "thread2"));
    t1.start();
    t2.start();



    printf("tid = %d, %p name = %s\n",
           handsome::current_thread::tid(),
           &handsome::thread_local_singleton<test>::instance(),
           handsome::thread_local_singleton<test>::instance().name().c_str());

    t1.join();

    t2.join();

    return 0;
}