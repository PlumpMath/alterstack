/*
 * Copyright 2015-2017 Alexey Syrnikov <san@masterspline.net>
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

#include "alterstack/bg_thread.hpp"

#include "alterstack/atomic_guard.hpp"
#include "alterstack/bg_runner.hpp"
#include "alterstack/scheduler.hpp"
#include "alterstack/task.hpp"
#include "alterstack/runner_info.hpp"
#include "alterstack/logger.hpp"
#include "alterstack/os_utils.hpp"

namespace alterstack
{
::std::atomic<uint32_t> BgThread::m_sleep_count;

void BgThread::thread_function()
{
    AtomicReturnBoolGuard thread_stopped_guard(m_thread_stopped);
    Scheduler::set_bg_runner({});
    os::set_thread_name();
    LOG << "BgThread::thread_function: started\n";

    while( true )
    {
        LOG << "BgThread::thread_function: will try to schedule something...\n";
        Scheduler::schedule( Scheduler::get_current_task() );
        if( is_stop_requested() )
            return;

        LOG << "BgThread::thread_function: waiting...\n";
        wait();
        LOG << "BgThread::thread_function: waked up\n";

        if( is_stop_requested() )
            return;
    }
}

void BgThread::ensure_thread_stopped()
{
    while( !m_thread_stopped.load() )
    {
        LOG << "BgThread::~BgThread(): waiting thread_function to stop\n";
        std::this_thread::sleep_for(::std::chrono::microseconds(1));
        wake_up();
    }
}

void BgThread::wait()
{
    m_sleep_count.fetch_add(1, std::memory_order_relaxed);
    m_task_avalable_futex.wait();
    m_sleep_count.fetch_sub(1, std::memory_order_relaxed);

}

BgThread::BgThread(Scheduler *scheduler)
    :scheduler_(scheduler)
{
    LOG << "BgThread::BgThread\n";
    m_stop_requested.store(false, ::std::memory_order_relaxed);
    m_thread_stopped.store(false, ::std::memory_order_release);
    m_os_thread = ::std::thread(&BgThread::thread_function, this);
}

BgThread::~BgThread()
{
    stop_thread();
    m_os_thread.join();
    LOG << "BgThread finished\n";
}

void BgThread::stop_thread()
{
    request_stop();
    wake_up();
    ensure_thread_stopped();
}

void BgThread::wake_up()
{
    m_task_avalable_futex.notify_all();
}

}
