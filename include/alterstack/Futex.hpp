/*
 * Copyright 2015 Alexey Syrnikov <san@masterspline.net>
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

#pragma once

#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <climits>

#include <atomic>
/**
 * @brief Class to implement wait() and notify() for OS thread in lock free way
 *
 * Worker OS thread processing data from one or many lock free queues (or other structures)
 * need to wait if there is no more "work" to do. Also it requires some way to wake up
 * if new work become ready. Futex implementing this behavior. Usage:
 * Producer after enqueueing new work need to call notify() on futex to wake up worker thread.
 * Consumer running in loop check queues for "work", if nothing call Futex::wait()
 * Usage:
 * @code{.cpp}
 * int ponsumer()
 * {
 *   while( !stop_the_work )
 *     if( check_all_queues() == EMPTY )
 *       futex.wait();
 *     else
 *       process_data();
 * }
 * int producer()
 * {
 *   while( true )
 *   {
 *     ...
 *     enqueue_data();
 *     futex.notify();
 *     ...
 *   }
 * }
 * @endcode
 *
 * Futex Algorithm:
 * wait() will set variable work_avalable to "false"
 *   if it was "false" current thread will increment wait_counter
 *     and call syscall futex(FUTEX_WAIT)
 *   else it will return (so that if some producer thread enqueued "work"
 *                        then current thread will check all queues again and call wait()
 *                        after that checks)
 *   after wake up worker thread will decrement wait_counter
 * notify() will set work_avalable to "true" and wake up sleeping worker thread
 *     if there is some (wait_counter > 0) by calling futex(FUTEX_WAKE)
 *
 * So in heavy loaded case no workers thread will call wait() and producer threads will
 * call notify() wich will just do read two variables work_avalable and wait_counter.
 */
class Futex
{
public:
    Futex() = default;
    Futex(const Futex&) = delete;
    Futex(Futex&&)      = delete;
    Futex& operator=(const Futex&) = delete;
    Futex& operator=(Futex&&) = delete;
    ~Futex() = default;

    void wait();
    void notify(int32_t count = 1);
    void notify_all();

private:
    std::atomic<int>      m_work_avalable{ 1 }; ///!< os syscall futex variable
    std::atomic<uint32_t> m_wait_counter { 0 }; ///!< sleeping threads counter
};
/**
 * @brief wait on futex (called by worker thread, if it have nothing to process)
 *
 * this function will wait until so other thread (consumer) will call notify()
 * It is thread safe, so if some other thread will notify() at same time,
 * current thread will not go to sleep or it will wake up immediately
 * (or some other thread waiting on this Futex will wake up)
 */
inline void Futex::wait()
{
    bool have_work = m_work_avalable.load(std::memory_order_acquire);
    if( have_work != 0 )
    {
        have_work = m_work_avalable.exchange( 0, std::memory_order_release );
        if( have_work != 0 )
            return;
    }
    /*
     * int futex(int *uaddr, int op, int val, const struct timespec *timeout,
     *           int *uaddr2, int val3);
     */
    m_wait_counter.fetch_add( 1, std::memory_order_relaxed);
    syscall(SYS_futex, &m_work_avalable, FUTEX_WAIT, 0, NULL, NULL, 0 );
    m_wait_counter.fetch_sub( 1, std::memory_order_release );

    return;
}
/**
 * @brief wake up thread waiting on this futex if there is some
 *
 * if no threads is waiting this function just read two variables
 * @param count how many threads to wake up (default 1)
 */
inline void Futex::notify(int32_t count)
{
    if( m_work_avalable.load(std::memory_order_acquire) == 0 )
    {
        m_work_avalable.store( 1, std::memory_order_release );
    }
    if( m_wait_counter.load( std::memory_order_acquire ) > 0 )
        syscall(SYS_futex, &m_work_avalable, FUTEX_WAKE, count, NULL, NULL, 0 );
}
/**
 * @brief wake up all threads waiting on this Futex
 */
inline void Futex::notify_all()
{
    notify( INT_MAX );
}
