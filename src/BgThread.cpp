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

#include "alterstack/BgThread.hpp"

#include <sys/prctl.h>

#include "alterstack/AtomicGuard.hpp"
#include "alterstack/BgRunner.hpp"
#include "alterstack/Scheduler.hpp"
#include "alterstack/Task.hpp"
#include "alterstack/Logger.hpp"

namespace alterstack
{
::std::atomic<uint32_t> BgThread::m_sleep_count;

void BgThread::thread_function()
{
    m_thread_started.store(true, ::std::memory_order_release);
    AtomicReturnBoolGuard thread_stopped_guard(m_thread_stopped);
    Scheduler::create_native_task_for_current_thread();
    Scheduler::m_thread_info->native_runner = false;

    static const char* name = "BgThread";
    ::prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));

    LOG << "BgThread::thread_function: started\n";

    while( true )
    {
        Task* next_task = scheduler_->get_next_from_queue();
        if( next_task != nullptr )
        {
            LOG << "BgThread::thread_function: got new task, switching on " << next_task << "\n";
            scheduler_->switch_to(next_task);
        }

        ::std::unique_lock<std::mutex> task_ready_guard(m_task_avalable_mutex);
        if( is_stop_requested_no_lock() )
        {
            return;
        }
        LOG << "BgThread::thread_function: waiting...\n";
        wait_on_cv(task_ready_guard);
        LOG << "BgThread::thread_function: waked up\n";
        if( is_stop_requested_no_lock() )
        {
            return;
        }
        task_ready_guard.unlock();
    }
}

void BgThread::ensure_thread_started()
{
    while(!m_thread_started.load())
    {
        LOG << "BgThread::~BgThread(): waiting thread_function to start\n";
        std::this_thread::yield();
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

void BgThread::wait_on_cv(::std::unique_lock<std::mutex>& task_ready_guard)
{
    m_sleep_count.fetch_add(1, std::memory_order_relaxed);
    m_task_avalable.wait_for(
                task_ready_guard
                ,::std::chrono::milliseconds(1000)); // fallback for losted notify
    m_sleep_count.fetch_sub(1, std::memory_order_relaxed);

}

BgThread::BgThread(Scheduler *scheduler)
    :scheduler_(scheduler)
    ,m_stop_requested(false)
{
    LOG << "BgThread::BgThread\n";
    m_thread_started.store(false, ::std::memory_order_relaxed);
    m_thread_stopped.store(false, ::std::memory_order_release);
    m_thread = ::std::thread(&BgThread::thread_function, this);
}

BgThread::~BgThread()
{
    stop_thread();
    m_thread.join();
    LOG << "Processor finished\n";
}

void BgThread::stop_thread()
{
    request_stop();
    wake_up();
    ensure_thread_started();
    ensure_thread_stopped();
}

void BgThread::wake_up()
{
    m_task_avalable.notify_one();
}

}
