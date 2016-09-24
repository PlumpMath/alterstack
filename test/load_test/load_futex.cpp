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

#include <string.h>
#include <errno.h>

#include <climits>
#include <atomic>
#include <thread>

#include <iostream>

#include "alterstack/Futex.hpp"

class TestLockFreeQueue
{
public:
    // returns 0 - enqueue failed (queue is "full"), != 0 succeeded (new counter value)
    uint32_t enqueue( uint32_t value = 1)
    {
        uint32_t counter = m_counter.load( std::memory_order_relaxed );
        if( counter > (UINT32_MAX - value))
            return 0;
        while( !m_counter.compare_exchange_weak(
                   counter, counter + value, std::memory_order_release ) )
        {
            if( counter > (UINT32_MAX - value) )
                return 0;
        }
        return counter + value;
    }
    // ret 0 - dequeue failed (queue is "empty"), != 0 - "dequeued" value (old counter value)
    uint32_t dequeue()
    {
        uint32_t counter = m_counter.load( std::memory_order_relaxed );
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

static std::atomic<bool> stop_requested{false};

static uint32_t batch_size = 2048;

void producer_function( Futex& futex, TestLockFreeQueue& queue )
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    if( pthread_setaffinity_np(
                pthread_self(), sizeof(cpu_set_t), &cpuset) < 0 )
    {
        printf("error pthread_setaffinity_np() to 0's CPU, %s", strerror(errno));
        exit( EXIT_FAILURE );
    }
    std::cout << "producer started\n";

    uint32_t q_value;
    while( !stop_requested )
    {
        if( (q_value = queue.enqueue(batch_size)) == 0 )
        {
            std::cout << "producer got 0 (queue is full)\n";
            usleep(100000);
            continue;
        }
        // std::cout << q_value << " after batch enqueue\n";
        futex.notify();
        usleep(1);
        uint32_t i = 0;
        while( !stop_requested && queue.enqueue() != 1 )
        {
            usleep(1);
            ++i;
        }
        futex.notify();
        if( i == 1 ) // increase batch_size if exhausted too quick
        {
            batch_size *= 2;
            std::cout << "New batch_size " << batch_size << std::endl;
        }
        if( i > 20 ) // decrease batch_size if it's too big
        {
            batch_size /= 2;
            std::cout << "New batch_size " << batch_size << std::endl;
        }
        while( (q_value = queue.enqueue(0) ) != 0 )
        {   // some data remain after last notify
            // consumer will wake up and exhaust it
            // if not - it's stalled
            for( int i = 0; i < 1000; ++i)
            {
                usleep(1);
                if( (q_value = queue.enqueue(0)) == 0 )
                    break;
            }
            if( q_value == 0 )
                break;
            if( stop_requested )
                break;
            std::cout << "after 1000 mks delay current q_value " << q_value << std::endl;
            usleep(100000);
        }
        // std::cout << i << " enqueue iterations for " << batch_size << " batch_size\n";
    }
}
void consumer_function( Futex& futex, TestLockFreeQueue& queue )
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET( 1, &cpuset);
    if( pthread_setaffinity_np(
                pthread_self(), sizeof(cpu_set_t), &cpuset) < 0 )
    {
        printf("error pthread_setaffinity_np() to 1's CPU, %s", strerror(errno));
        exit( EXIT_FAILURE );
    }
    std::cout << "consumer started\n";
    uint32_t q_value;
    while( !stop_requested )
    {
        if( (q_value = queue.dequeue()) == 0 )
        {
            futex.wait();
            continue;
        }
    }
}
/**
 * @brief main will load test Futex miplementation
 *
 * it starts two threads: one producer and one consumer
 * Consumer will dequeue "work" from lock free queue and process it. If queue is
 * empty - consumer will call Futex::wait() to sleep on futex.
 *
 * Producer will batch create some "work" in lock free queue and Futex::notify(),
 * then wait for queue will become empty (trying to catch point when consumer
 * change from working mode to sleep mode), then enqueue some more work and
 * call Futex::notify() to wake up consumer. After that it checks that queue will
 * be processed by consumer (was it sleeping or going to sleep at notify()).
 * Producer and Consumer is running on different OS threads.
 *
 * The aim of this test is to check, that notify will always work (whether consumer waiting()
 * or not)
 *
 * @return 0 on success
 */
int main()
{
    std::cout << "main started\n";
    TestLockFreeQueue queue;
    Futex futex;
    std::cout << "main will create producers\n";
    std::thread producer1{producer_function, std::ref(futex), std::ref(queue)};
    std::cout << "main will create consumers\n";
    std::thread consumer1{consumer_function, std::ref(futex), std::ref(queue)};
    std::cout << "main will sleep\n";
    sleep( 100 );
    std::cout << "main will notify\n";
    stop_requested = true;
    futex.notify_all();
    std::cout << "main will join consumer\n";
    consumer1.join();
    std::cout << "main will join producers\n";
    producer1.join();
    std::cout << "main will finish\n";
    return 0;
}
