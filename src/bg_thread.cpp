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

#include "alterstack/bg_thread.h"

#include "alterstack/atomic_guard.h"
#include "alterstack/bg_runner.h"
#include "alterstack/scheduler.h"
#include "alterstack/task.h"
#include "alterstack/logger.h"
#include "alterstack/os_utils.h"

namespace alterstack
{
::std::atomic<uint32_t> BgThread::m_sleep_count;

void BgThread::thread_function()
{
    AtomicReturnBoolGuard thread_stopped_guard(m_thread_stopped);
    Scheduler::create_native_task_for_current_thread( RunnerType::BgRunner );

    os::set_thread_name();

    LOG << "BgThread::thread_function: started\n";

    while( true )
    {
        Task* next_task = scheduler_->get_next_from_queue();
        if( next_task != nullptr )
        {
            LOG << "BgThread::thread_function: got new task, switching on " << next_task << "\n";
            scheduler_->switch_to(next_task);
        }

        if( is_stop_requested() )
        {
            return;
        }
        LOG << "BgThread::thread_function: waiting...\n";
        wait();
        LOG << "BgThread::thread_function: waked up\n";
        if( is_stop_requested() )
        {
            return;
        }
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
