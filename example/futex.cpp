/*
 * Copyright 2016 Alexey Syrnikov <san@masterspline.net>
 *
 * This file is part of Alterstack.
 *
 * Alterstack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alterstack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Alterstack.  If not, see <http://www.gnu.org/licenses/>
 */

#include <climits>
#include <atomic>
#include <thread>

#include <iostream>

#include "Futex.hpp"

class LockFreeTestQueue
{
public:
    // 0 enqueue failed (queue is "full"), != 0 succeeded (new counter value)
    uint32_t enqueue()
    {
        uint32_t counter = m_counter.load( std::memory_order_acquire );
        if( counter == UINT32_MAX )
            return 0;
        while( !m_counter.compare_exchange_weak(
                   counter, counter + 1, std::memory_order_relaxed ) )
        {
            if( counter == UINT32_MAX )
                return 0;
        }
        return counter + 1;
    }
    // 0 dequeue failed (queue is "empty"), != 0 - "dequeued" value (old counter value)
    uint32_t dequeue()
    {
        uint32_t counter = m_counter.load( std::memory_order_acquire );
        if( counter == 0 )
            return 0;
        while( !m_counter.compare_exchange_weak(
                   counter, counter - 1, std::memory_order_relaxed ) )
        {
            if( counter == 0 )
                return 0;
        }
        return counter;
    }

private:
    std::atomic<uint32_t> m_counter{0};
};

static volatile bool stop_requested{false};

void producer_function( Futex& futex, LockFreeTestQueue& queue )
{
    std::cout << "producer started\n";
    uint32_t q_value;
    while( !stop_requested )
    {
        if( (q_value = queue.enqueue()) == 0 )
        {
            std::cout << "producer got 0 (queue is full)\n";
            usleep(100000);
            continue;
        }
        std::cout << "producer enqueued " << q_value << "\n";
        futex.notify();
        usleep(100000);
    }
}
void consumer_function( Futex& futex, LockFreeTestQueue& queue )
{
    std::cout << "consumer started\n";
    uint32_t q_value;
    while( !stop_requested )
    {
        if( (q_value = queue.dequeue()) == 0 )
        {
            std::cout << "consumer got 0 (queue is empty)\n";
            futex.wait();
            continue;
        }
        std::cout << "consumer got " << q_value << ", processing\n";
    }
}

int main()
{
    std::cout << "main started\n";
    LockFreeTestQueue queue;
    Futex futex;
    std::cout << "main will create producers\n";
    std::thread producer1{producer_function, std::ref(futex), std::ref(queue)};
    std::thread producer2{producer_function, std::ref(futex), std::ref(queue)};
    std::cout << "main will create producers\n";
    std::thread consumer1{consumer_function, std::ref(futex), std::ref(queue)};
    std::cout << "main will sleep\n";
    sleep( 1 );
    std::cout << "main will notify\n";
    stop_requested = true;
    futex.notify_all();
    std::cout << "main will join consumer\n";
    consumer1.join();
    std::cout << "main will join producers\n";
    producer1.join();
    producer2.join();
    std::cout << "main will finish\n";
    return 0;
}
