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

#include "alterstack/scheduler.hpp"
#include "alterstack/logger.hpp"

namespace alterstack
{

namespace ctx = ::scontext;

TaskBase::TaskBase( bool thread_bound )
    :m_is_thread_bound{ thread_bound }
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
    LOG << "Task::Task\n";
    m_context = ctx::make_fcontext( m_stack->stack_top(), m_stack->size(), _run_wrapper);
    LOG << "Task::Task: m_context " << m_context << "\n";

    Scheduler::run_new_task( this );
}

Task::~Task()
{
    LOG << "~Task(): unbound task " << this << "\n";
    release();
    while( m_state != TaskState::Finished
           || m_context == nullptr )
    {
        LOG << "~Task() state " << static_cast<uint32_t>(m_state.load())
            << " context " << m_context << "\n";
        yield();
        ::std::this_thread::yield();
    }
}
/**
 * @brief constructor to create thread bound Task
 *
 * @param native_info points to RunnerInfo
 */

TaskBase::TaskBase( Passkey<TaskRunner> )
    :m_context(nullptr)
    ,m_state( TaskState::Running )
    ,m_is_thread_bound{ true }
{}
/**
 * @brief destructor will wait if Task still Running
 */
TaskBase::~TaskBase()
{
    LOG << "TaskBase::~TaskBase: " << this << "\n";
    release();
    m_state = TaskState::Finished;  // unbound Task will be marked as Clear in _run_wrapper()
    return;
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
        LOG << "Task::wait: Task finished, nothing to wait\n";
        return;
    }
    LOG << "Task::wait: running Awaitable::wait()\n";
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
            LOG << "Task::_run_wrapper: started\n";
            Scheduler::post_jump_fcontext( {}, transfer, current );

            current->m_runnable();
            LOG << "Task::_run_wrapper: runnable finished, cleaning Task\n";
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
        LOG << "Task::_run_wrapper: Got exception:\n" << ex.what();
    }
    catch(...)
    {
        LOG << "Task::_run_wrapper: Got strange exception\n";
    }
    ::exit(EXIT_FAILURE);
}

}
