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

#include "alterstack/scheduler.hpp"

#include <string>
#include <thread>

#include "alterstack/stack.hpp"
#include "alterstack/spin_lock.hpp"
#include "alterstack/context.hpp"
#include "alterstack/bg_runner.hpp"
#include "alterstack/task_runner.hpp"
#include "alterstack/logger.hpp"

namespace alterstack
{

namespace ctx = ::scontext;

Scheduler::Scheduler()
    :bg_runner_(this)
    ,running_queue_()
{}
/**
 * @brief schedule next task on current OS thread
 *
 * schedule() tries to find next Running Task using scheduling algorithm of Scheduler.
 * If new task available switch OS thread to it. If no Task available
 * do nothing (continue running current Task).
 * @return true if at least one switch done
 */
bool Scheduler::schedule(Task *current_task)
{
    return instance().do_schedule( current_task );
}

bool Scheduler::do_schedule( Task *current_task )
{
    LOG << "Scheduler::do_schedule( current_task )\n";

    Task* next_task = nullptr;
    while( true )
    {
        next_task = get_next_task( current_task );
        if( next_task != nullptr )
        {
            break;
        }
        else
        {
            if( current_task->state({}) == TaskState::Running )
            {
                LOG << "Scheduler::do_schedule: nowhere to switch, do nothing\n";
                return false;
            }
            else // Only bound Common current_task will get here
            {
                // FIXME: here I need Task::thread_wait() method to wait and exit on thread cancelled
                if( current_task->m_state == TaskState::Waiting )
                {
                    LOG << "Scheduler::do_schedule: current Task is in Waiting state and "
                           "nowhere to switch, waiting...\n";
                    TaskRunner::current().native_futex.wait();
                }

            }
        }
    }
    switch_to( next_task );
    return true;
}
/**
 * @brief switch OS thread to newly created Task, current task stay Running
 * @param task new task to run
 */
void Scheduler::run_new_task( Task* task )
{
    instance().do_schedule_new_task(task);
}

void Scheduler::do_schedule_new_task( Task* task )
{
    switch_to(task);
}
/**
 * @brief switch current task to new and store old task in running if required
 * @param new_task next task to run
 * @param old_task_state running state of old task
 *
 * old_task_state == Task::RunState::Running current task will be stored in
 * running queue or in native
 * old_task_state == Task::RunState::Waiting current task olready stored in waiting queue
 * old_task_state == Task::RunState::Finished current task can be destroied, do not use it
 *
 * Switching tasks consist of two steps:
 *
 * 1. switch stack (context) to new one
 * 2. store old task in running queue if old still running
 * Last step done in post_switch_fixup() which is part of switch_to().
 */
void Scheduler::switch_to( Task* new_task )
{
    LOG << "Scheduler::switch_to\n";
    Task* old_task = get_current_task();
    LOG << "Scheduler::switch_to old_task -> new_task(m_context): "
        << old_task << " -> " << new_task << " (" << new_task->m_context << ")\n";
    TaskRunner::set_current_task( new_task );
    ::scontext::transfer_t transfer = ::scontext::jump_fcontext(
                new_task->m_context
                ,(void*)old_task );
    // as result for jump_fcontext some new contexts will work
    // and after that control will come back in this context exactly in
    // it's state before jump (so old context become current again
    // And it will be Running as before and because someone switched to us -
    // no reason to switch to Waiting context)
    // Here exist 3 contexts:
    // old - is context I switching from
    // new - is context I switched to
    // prev - is context I came back from (after new context, and other
    // did it's work)

    post_jump_fcontext( transfer, old_task );
}
/**
 * @brief store old task in running queue, if it is not nullptr and is AlterNative
 * @param old_task task to store
 */
void Scheduler::post_jump_fcontext( ::scontext::transfer_t transfer, Task* current_task )
{
    LOG << "Scheduler::post_jump_fcontext\n";

    current_task->m_context = nullptr;
    Task* prev_task = (Task*)transfer.data;
    if( prev_task->m_state.load( std::memory_order_relaxed ) == TaskState::Finished )
    {
        prev_task->m_state.store( TaskState::Clear, std::memory_order_release );
        LOG << "Scheduler::post_jump_fcontext prev::m_state = Finished\n";
    }
    else
    {
        prev_task->m_context = transfer.fctx;
        LOG << "Scheduler::post_jump_fcontext saved prev_task m_context " << transfer.fctx << "\n";
    }

    if( !prev_task->is_thread_bound()
            && prev_task->m_state == TaskState::Running )
    {
        LOG << "Scheduler::post_jump_fcontext: enqueueing old task\n";
        enqueue_alternative_task(prev_task);
    }
}
/**
 * @brief get current Task*
 * @return pointer to current Task
 */
Task* Scheduler::get_current_task()
{
    if ( TaskRunner::current_task() == nullptr )
    {
        TaskRunner::set_current_task( TaskRunner::native_task() );
    }
    return TaskRunner::current_task();
}
/**
 * @brief get Native Task pointer (create Task instance if does not exist)
 * @return Task* to Native Task instance
 */
Task* Scheduler::get_native_task()
{
    return TaskRunner::native_task();
}
/**
 * @brief get Task* from running queue
 * @return Task* or nullptr if queue is empty
 */
Task* Scheduler::get_running_from_queue() noexcept
{
    bool have_more_tasks = false;
    Task* task = running_queue_.get_item(have_more_tasks);
    LOG << "Scheduler::get_next_from_queue: got task " << task << " from running queue\n";
    if( task != nullptr
            && have_more_tasks)
    {
        bg_runner_.notify();
    }
    return task;
}
/**
 * @brief get Native Task* if it is running or nullptr
 * @return Native Task* or nullptr
 */
Task* Scheduler::get_running_from_native()
{
    if( TaskRunner::native_task()->m_state == TaskState::Running )
    {
        LOG << "Scheduler::_get_next_from_native: got native task\n";
        return TaskRunner::native_task();
    }
    return nullptr;
}
/**
 * @brief enqueue task in running queue
 * @param task task to store
 * get_next_from_native() is threadsafe
 */
void Scheduler::enqueue_alternative_task(Task *task) noexcept
{
    assert(task != nullptr);
    auto& scheduler = instance();
    scheduler.running_queue_.put_item(task);
    LOG << "Scheduler::enqueue_alternative_task: task " << task
        << " stored in running task queue\n";
    scheduler.bg_runner_.notify();
}
/**
 * @brief get next Task* to run using schedule algorithm of Scheduler
 * @return next running Task* or nullptr
 */
Task *Scheduler::get_next_task( Task *current_task )
{
    Task* next_task = nullptr;
    if( current_task->is_thread_bound() ) // Common or BgRunner thread in it's own context
    {
        LOG << "Scheduler::next_task: Common thread code want to switch\n";
        next_task = get_running_from_queue();
    }
    else // unbound Task in Common thread or in BgRunner
    {
        if( TaskRunner::current().type() == RunnerType::CommonThread )
            // Common code in unbound context
        {
            LOG << "Scheduler::next_task: in unbound context\n";
            next_task = get_running_from_native();
            if( next_task == nullptr )
                next_task = get_running_from_queue();
        }
        else // BgRunner in unbound context
        {
            LOG << "Scheduler::next_task: in BgRunner\n";
            next_task = get_running_from_queue();
        }
        auto current_state = current_task->state({});
        if( next_task == nullptr
                && ( current_state == TaskState::Finished
                     || current_state == TaskState::Waiting ) )
        {
            LOG << "Scheduler::next_task: current_task is unbound in Finished state\n";
            next_task = get_native_task();
        }
    }
    return next_task;
}

/**
 * @brief make task (Native and AlterNative) running and schedule it to execution
 *
 * task must NOT be in any queue (because of multithreading)
 * @param task pointer to running Task
 */
void Scheduler::add_running_task( Task* task ) noexcept
{
    task->m_state = TaskState::Running;
    if( task->is_thread_bound() )
    {
        LOG << "Scheduler::enqueue_running_task: task " << task <<
               " is Native, marking Ready and notifying\n";
        TaskRunner::current().native_futex.notify();
    }
    else // AlterNative or BgRunner
    {
        LOG << "Scheduler::enqueue_running_task: task " << task <<
               " is AlterNative, enqueueing in running queue\n";
        Scheduler::enqueue_alternative_task(task);
    }
}

}
