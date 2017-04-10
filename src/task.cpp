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

#include "alterstack/task.hpp"

#include <cstdlib>

#include <thread>
#include <stdexcept>
#include <iostream>

#include "alterstack/scheduler.hpp"

namespace alterstack
{

namespace ctx = ::scontext;

TaskBase::TaskBase( bool is_thread_bound )
    :m_is_thread_bound{ is_thread_bound }
{}

TaskBase::~TaskBase()
{}

/**
 * @brief constructor to create thread unbound Task
 * @param runnable void() function or functor to start
 */
Task::Task( ::std::function<void()> runnable )
    :TaskBase{ false }
    ,m_stack{ new Stack() }
    ,m_runnable{ std::move(runnable) }
{
    m_context = ctx::make_fcontext( m_stack->stack_top(), m_stack->size(), _run_wrapper);

    Scheduler::run_new_task( this );
}

Task::~Task()
{
    release();
    while( m_state != TaskState::Finished
           || m_context == nullptr )
    {
        yield();
        ::std::this_thread::yield();
    }
}
/**
 * @brief constructor to create thread bound Task
 */
BoundTask::BoundTask(Passkey<TaskRunner> , TaskRunner* runner)
    :TaskBase{ true }
    ,m_task_runner{ runner }
{}

/**
 * @brief destructor will wait if Task still Running
 */

BoundTask::~BoundTask()
{
    release();
    m_state = TaskState::Finished;  // unbound Task will be marked as Clear in _run_wrapper()
    return;
}

void BoundTask::notify()
{
    m_task_runner->native_futex.notify();
}
/**
 * @brief yield current Task, schedule next (if avalable), current stay running
 */
void Task::yield()
{
    Scheduler::schedule() ;
}

/**
 * @brief switch caller Task in Waiting state while this is Running
 *
 * If this already finished return immediately
 */
void TaskBase::join()
{
    if( m_state == TaskState::Finished )
    {
        return;
    }
    m_awaitable.wait();
}
/**
 * @brief helper function to start Task's runnable object and clean when it's finished
 * @param task_ptr pointer to Task instance
 */
void Task::_run_wrapper( ::scontext::transfer_t transfer ) noexcept
{
    try
    {
        Task* current = static_cast<Task*>( Scheduler::get_current_task() );
        {
            Scheduler::post_jump_fcontext( {}, transfer, current );

            current->m_runnable();
            current->release();
            current->m_state.store( TaskState::Finished, std::memory_order_release );
        } // here all local objects NUST be destroyed because schedule() will never return
        Scheduler::schedule( current );
        // This line is unreachable,
        // because current context will never scheduled
        __builtin_unreachable();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "_run_wrapper got exception " << ex.what() << "\n";
    }
    catch(...)
    {
        std::cerr << "_run_wrapper got strange exception\n";
    }
    ::exit(EXIT_FAILURE);
}

}
