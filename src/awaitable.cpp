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

#include "alterstack/awaitable.hpp"

#include "alterstack/scheduler.hpp"
#include "alterstack/task_runner.hpp"

#include <cassert>

namespace alterstack
{

Awaitable::~Awaitable()
{
    AwaitableData aw_data = ::std::atomic_load_explicit(&m_data,::std::memory_order_acquire);
    if( aw_data.is_finished
            || aw_data.head == nullptr )
    {
        return;
    }
    wait();
}

bool Awaitable::insert_current_task_in_waitlist()
{
    TaskBase* const current_task = Scheduler::get_current_task();
    AwaitableData aw_data = ::std::atomic_load_explicit(&m_data,::std::memory_order_acquire);
    if( aw_data.is_finished )
    {
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
    current_task->set_next( aw_data.head );
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
            current_task->m_context = (void*)0x01;
            return false;
        }
        current_task->set_next( aw_data.head );
    }
    return true;
}

void Awaitable::wait()
{
    if( insert_current_task_in_waitlist() )
    {
        Scheduler::schedule();
    }
}

void Awaitable::release()
{
    AwaitableData aw_data = ::std::atomic_load_explicit(&m_data,::std::memory_order_acquire);
    if( aw_data.is_finished )
    {
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
            return;
        }
    }

    TaskBase* task_list = aw_data.head;
    Scheduler::add_waiting_list_to_running( {}, task_list );
}

}
