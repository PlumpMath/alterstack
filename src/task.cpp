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

/**
 * @brief constructor to create AlterNative Task
 * @param runnable void() function or functor to start
 */
Task::Task( ::std::function<void()> runnable )
    :m_native_info{ nullptr }
    ,m_state{ TaskState::Running }
    ,m_stack{ new Stack() }
    ,m_runnable{ std::move(runnable) }
{
    LOG << "Task::Task\n";
    m_context = ctx::make_fcontext( m_stack->stack_top(), m_stack->size(), _run_wrapper);
    LOG << "Task::Task: m_context " << m_context << "\n";

    Scheduler::run_new_task( this );
}
/**
 * @brief constructor to create Native Task
 *
 * @param native_info points to RunnerInfo
 */
Task::Task(RunnerInfo* native_info)
    :m_context(nullptr)
    ,m_native_info(native_info)
    ,m_state(TaskState::Running)
{}
/**
 * @brief destructor will wait if Task still Running
 */
Task::~Task()
{
    LOG << "Task::~Task: " << this << "\n";
    if( is_thread_bound() ) // AlterNative Task marked Created in _run_wrapper()
    {
        m_state = TaskState::Finished;
    }
    wait();
    while( m_state != TaskState::Finished )
    {
        yield();
        if( m_state != TaskState::Finished )
        {
            ::std::this_thread::yield();
        }
    }
}
/**
 * @brief yield current Task, schedule next (if avalable), current stay running
 */
void Task::yield()
{
    Scheduler::schedule();
}

/**
 * @brief switch caller Task in Waiting state while this is Running
 *
 * If this already finished return immediately
 */
void Task::wait()
{
    if( m_state == TaskState::Finished )
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
    ::exit(EXIT_FAILURE);
}

}
