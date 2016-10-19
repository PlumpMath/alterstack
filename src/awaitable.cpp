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

#include "alterstack/awaitable.h"
#include "alterstack/scheduler.h"
#include "alterstack/logger.h"

#include <cassert>

namespace alterstack
{

Awaitable::~Awaitable()
{
    AwaitableData aw_data = ::std::atomic_load_explicit(&m_data,::std::memory_order_acquire);
    LOG << "Awaitable::~Awaitable: Awaitable m_next " << aw_data.head << "\n";
    LOG << "Awaitable::~Awaitable: Awaitable m_state " << aw_data.is_finished << "\n";
    if( aw_data.is_finished
            || aw_data.head == nullptr )
    {
        LOG << "Awaitable::~Awaitable: Running awaitable finished or next == nullptr\n";
        return;
    }
    LOG << "Awaitable::~Awaitable: have waiters, will wait";
    wait();
}

bool Awaitable::insert_current_task_in_waitlist()
{
    Task* const current_task = Scheduler::get_current_task();
    AwaitableData aw_data = ::std::atomic_load_explicit(&m_data,::std::memory_order_acquire);
    LOG << "Awaitable::wait: current Task* " << current_task << "\n";
    LOG << "Awaitable::wait: Awaitable m_next " << aw_data.head << "\n";
    LOG << "Awaitable::wait: Awaitable m_state " << (int)aw_data.is_finished << "\n";
    if( aw_data.is_finished )
    {
        LOG << "Awaitable::wait: Awaitable Finished, all done\n";
        return false;
    }
    // locking not needed because external task can change state
    // only from Waiting to Running at wakeup but current_task still not in wait queue
    current_task->m_state = TaskState::Waiting;
    // Task* current_task will be placed in wait list and small time later
    // it will switch to other task but if this task is AlterNative
    // in this tiny time it can be woken up, moved in running queue and executed
    // m_context = nullptr to protect from switching to it
    current_task->m_context = nullptr;
    current_task->next_ = aw_data.head;
    AwaitableData new_aw_data;
    new_aw_data.head = current_task;
    while(!::std::atomic_compare_exchange_weak_explicit(
              &m_data
              ,&aw_data
              ,new_aw_data
              ,::std::memory_order_release
              ,::std::memory_order_acquire) )
    {
        if( aw_data.is_finished )
        {
            // locking not needed because external task can change state
            // only from Waiting to Running at wakeup but current_task still not in wait queue
            current_task->m_state = TaskState::Running;
            LOG << "Awaitable::wait: Awaitable Finished, all done\n";
            current_task->m_context = (void*)0x01;
            return false;
        }
        current_task->next_ = aw_data.head;
    }
    LOG << "Awaitable::wait: current Task equeued\n";
    LOG << "Awaitable::wait: current Task* m_next " << current_task->next_ << "\n";
    LOG << "Awaitable::wait: Awaitable m_next " << new_aw_data.head << "\n";
    return true;
}

void Awaitable::wait()
{
    if( insert_current_task_in_waitlist() )
    {
        Scheduler::schedule_waiting_task();
    }
}

void Awaitable::release()
{
    AwaitableData aw_data = ::std::atomic_load_explicit(&m_data,::std::memory_order_acquire);
    LOG << "Awaitable::release: Awaitable m_next " << aw_data.head << "\n";
    LOG << "Awaitable::release: Awaitable m_state " << (int)aw_data.is_finished << "\n";
    if( aw_data.is_finished )
    {
        LOG << "Awaitable::release: Awaitable Finished, all done\n";
        return;
    }
    AwaitableData new_aw_data;
    new_aw_data.head = nullptr;
    new_aw_data.is_finished = true;
    while(!::std::atomic_compare_exchange_weak(
              &m_data
              ,&aw_data
              ,new_aw_data) )
    {
        if( aw_data.is_finished )
        {
            LOG << "Awaitable::release: Awaitable Finished, all done\n";
            return;
        }
    }
    LOG << "Awaitable::release: post CAS Awaitable m_next " << aw_data.head << "\n";

    Task* next_task = aw_data.head;
    while( next_task != nullptr )
    {
        Task* task = next_task;
        // after wakeing up task can be deleted before next iteration in this loop
        next_task = next_task->next_;
        if( task->is_native() )
        {
            LOG << "Awaitable::release: task " << task <<
                   " is Native, marking Ready and notifying\n";
            std::unique_lock<std::mutex> native_guard(task->m_native_info->native_mutex);
            task->m_state = TaskState::Running;
            native_guard.unlock();
            task->m_native_info->native_ready.notify_one();
        }
        else // AlterNative or BgRunner
        {
            LOG << "Awaitable::release: task " << task <<
                   " is AlterNative, enqueueing in running queue\n";
            task->m_state = TaskState::Running;
            Scheduler::enqueue_task(task);
        }
    }
}

}
