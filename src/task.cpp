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

#include "alterstack/task.h"

#include <thread>
#include <stdexcept>

#include "alterstack/scheduler.h"
#include "alterstack/logger.h"

namespace alterstack
{

namespace ctx = ::scontext;

Task::Task()
    :m_native_info(nullptr)
    ,m_state(TaskState::Created)
    ,m_stack(new Stack())
{}

Task::Task(RunnerInfo* native_info)
    :m_context(nullptr)
    ,m_native_info(native_info)
    ,m_state(TaskState::Running)
{}

Task::~Task()
{
    LOG << "Task::~Task: " << this << "\n";
    if( is_native() ) // AlterNative Task marked Created in _run_wrapper()
    {
        m_state = TaskState::Created;
    }
    wait();
    while( m_state != TaskState::Created )
    {
        yield();
        if( m_state != TaskState::Created )
        {
            ::std::this_thread::yield();
        }
    }
}

void Task::set_function() // FIXME: this hack will be fixed with extending runnable types
{
    m_context = ctx::make_fcontext( m_stack->stack_top(), m_stack->size(), _run_wrapper);
    LOG << "Task::set_function(): m_context " << m_context << "\n";
}

void Task::run( ::std::function<void()> runnable )
{
    LOG << "Task::run\n";
    if( m_state != TaskState::Created)
        throw std::logic_error("Task still running");

    m_state = TaskState::Running;
    m_runnable = runnable;

    Scheduler::schedule_new_task( this );
}

void Task::yield()
{
    Scheduler::schedule();
}

void Task::wait()
{
    if( m_state == TaskState::Created
            || m_state == TaskState::Finished )
    {
        LOG << "Task::wait: Task finished, nothing to wait\n";
        return;
    }
    LOG << "Task::wait: running Awaitable::wait()\n";
    awaitable_.wait();
}

void Task::_run_wrapper( ::scontext::transfer_t transfer ) noexcept
{
    try
    {
        Task* next_task;
        {
            LOG << "Task::_run_wrapper: started\n";
            // FIXME: check this logic, old context missed
            Task* prev_task = reinterpret_cast<Task*>(transfer.data);
            if( prev_task != nullptr )
            {
                prev_task->m_context = transfer.fctx;
                LOG << "Task::_run_wrapper saved prev_task m_context " << transfer.fctx << "\n";
            }
            Scheduler::post_switch_fixup(prev_task);

            Task* current = Scheduler::get_current_task();
            current->m_runnable();
            LOG << "Task::_run_wrapper: runnable finished, cleaning Task\n";

            current->m_state = TaskState::Finished;
            current->release();

            // _run_wrapper() used only in AlterNative(BgRunner) Task
            next_task = Scheduler::get_next_task();
            if(next_task == nullptr)
            {
                next_task = Scheduler::get_native_task();
            }
            current->m_state = TaskState::Created;
            assert(next_task != nullptr);
        }
        Scheduler::switch_to(next_task, TaskState::Finished); // exactly Finished

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
}

}
