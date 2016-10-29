/*
 * Copyright 2015-2016 Alexey Syrnikov <san@masterspline.net>
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

#include <atomic>
#include <thread>

#include "alterstack/futex.hpp"

namespace alterstack
{
class BgRunner;
class Scheduler;
/**
 * @brief Single Task background runner thread.
 */
class BgThread
{
public:
    BgThread(const BgThread&) = delete;
    BgThread(BgThread&&) = delete;
    BgThread& operator=(const BgThread&) = delete;
    BgThread& operator=(BgThread&&) = delete;
    BgThread() = delete;
    /**
     * @brief start one OS thread
     * @param bg_runner reference to BgRunner
     */
    explicit BgThread(Scheduler* scheduler);
    /**
     * @brief destructor stops OS thread, return when it's stopped
     */
    ~BgThread();
    /**
     * @brief ask this BgThread to stop
     *
     * do not wait for thread stop
     */
    void request_stop();
    /**
     * @brief stop thread function, returns when thread stopped
     */
    void stop_thread();
    void wake_up();
    /**
     * @brief get current waiting BgThread threads count
     * @return number of currently sleeping threads
     */
    static uint32_t sleep_count();

private:
    void thread_function();
    /**
     * @brief ensure that thread function stopped, wait until stop
     */
    void ensure_thread_stopped();
    bool is_stop_requested();
    void wait();

    Scheduler*        scheduler_;       //!< Scheduler reference
    std::thread       m_os_thread;         //!< OS thread
    std::atomic<bool> m_thread_stopped; //!< true if thread_function stopped
    std::atomic<bool> m_stop_requested; //!< true when current BgThread need to stop
    Futex             m_task_avalable_futex; //!< Futex to wait for new tasks

    static ::std::atomic<uint32_t> m_sleep_count;
};

inline void BgThread::request_stop()
{
    m_stop_requested.store( true, std::memory_order_release );
}

inline bool BgThread::is_stop_requested()
{
    return  __builtin_expect( m_stop_requested.load(std::memory_order_acquire), false );
}

inline uint32_t BgThread::sleep_count()
{
    return m_sleep_count.load(std::memory_order_acquire);
}

}
