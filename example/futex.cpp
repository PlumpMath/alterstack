#include <iostream>
#include <atomic>
#include <thread>

#include "Futex.hpp"

void thread_function( Futex& f )
{
    std::cout << "thread started\n";
    f.wait();
    std::cout << "first wait() passed\n";
    f.wait();
    std::cout << "second wait() passed, exiting thread\n";
    return;
}

int main()
{
    Futex fut;
    std::thread t{thread_function, std::ref(fut)};
    std::cout << "main will sleep\n";
    sleep(1);
    std::cout << "main will notify\n";
    fut.notify();
    std::cout << "main will join thread\n";
    t.join();
    std::cout << "main will finish\n";
    return 0;
}
