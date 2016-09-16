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

#include "alterstack/Scheduler.hpp"

#include <string>
#include <thread>

#include "alterstack/Stack.hpp"
#include "alterstack/SpinLock.hpp"
#include "alterstack/Context.hpp"
#include "alterstack/BgRunner.hpp"
#include "alterstack/Logger.hpp"

namespace alterstack
{

namespace ctx = ::scontext;

Scheduler::Scheduler()
    :bg_runner_(this)
    ,running_queue_()
{}

bool Scheduler::schedule(bool old_stay_running)
{
    return instance().do_schedule(old_stay_running);
}

bool Scheduler::do_schedule(bool old_stay_running)
{
    LOG << "Scheduler::schedule\n";

    Task* next_task = get_next_task();
    if( next_task == nullptr )
    {
        LOG << "Scheduler::schedule: nowhere to switch, do nothing\n";
        return false;
    }
    switch_to(next_task
              ,old_stay_running ? TaskState::Running : TaskState::Waiting);
    return true;
}
void Scheduler::schedule_new_task( Task* task )
{
    instance().do_schedule_new_task(task);
}

void Scheduler::do_schedule_new_task( Task* task )
{
    task->set_function(); // FIXME: this hack will be fixed with extending runnable types
    switch_to(task);
}

void Scheduler::switch_to(Task* new_task, TaskState old_task_state)
{
    LOG << "Scheduler::switch_to\n";
    Task* old_task;
    switch( old_task_state )
    {
    case TaskState::Running:
    case TaskState::Waiting:
        old_task = get_current_task();
        break;
    default:
        old_task = nullptr;
    }
    LOG << "Scheduler::switch_to old_task -> new_task"
        << old_task << " -> " << new_task << "\n";
    m_thread_info->current_task = new_task;
//    do not remember, why is this here
//    while( new_task->m_context == nullptr )
//    {
//        ::std::this_thread::yield();
//    }
    Task* prev_task;
    switch( old_task_state )
    {
    case TaskState::Running:
    { // FIXME: check logick for this method
        ::scontext::transfer_t transfer = ::scontext::jump_fcontext(
                    new_task->m_context
                    ,(void*)old_task);
        // as result for previous switch some new contexts will work
        // and after that I will come back in this context exactly in
        // it's state before jump (so old context become current again
        // And it will be Running as before and because someone switched to us -
        // no reason to switch to Waiting context)
        // Here exist 3 contexts:
        // old - is context I switching from
        // new - is context I switched to
        // prev - is context I came back from (after new context, and other
        // did it's work)
        prev_task = (Task*)transfer.data;
        if( prev_task != nullptr )
            prev_task->m_context = transfer.fctx;
        break;
    }
    case TaskState::Waiting:
    {
        ::scontext::transfer_t transfer = ::scontext::jump_fcontext(
                    new_task->m_context
                    ,(void*)nullptr);
        prev_task = (Task*)transfer.data;
        if( prev_task != nullptr )
            prev_task->m_context = transfer.fctx;
        break;
    }
    default:
    {
        ::scontext::transfer_t transfer = ::scontext::jump_fcontext(
                    new_task->m_context
                    ,nullptr);
        prev_task = (Task*)transfer.data;
        if( prev_task != nullptr )
            prev_task->m_context = transfer.fctx;
        break;
    }
    }

    post_switch_fixup(prev_task);
}

void Scheduler::post_switch_fixup(Task *prev_task)
{
    LOG << "Scheduler::post_switch_fixup\n";
    if( prev_task == nullptr )
    {
        LOG << "Scheduler::post_switch_fixup: old_task == nullptr, do nothing\n";
        return;
    }
    if( !prev_task->is_native() ) // AlterNative
    {
        LOG << "Scheduler::post_switch_fixup: enqueueing old task\n";
        enqueue_task(prev_task);
    }
}

Task *Scheduler::get_current_task()
{
    if( !m_thread_info )
    {
        m_thread_info.reset(new AsThreadInfo());
    }
    if (m_thread_info->current_task == nullptr )
    {
        if( !m_thread_info->native_task )
        {
            create_native_task_for_current_thread();
        }
        m_thread_info->current_task = m_thread_info->native_task.get();
    }
    return m_thread_info->current_task;
}

Task *Scheduler::get_native_task()
{
    if( !m_thread_info )
    {
        m_thread_info.reset(new AsThreadInfo() );
    }
    if( !m_thread_info->native_task )
    {
        create_native_task_for_current_thread();
    }
    return m_thread_info->native_task.get();
}

void Scheduler::create_native_task_for_current_thread()
{
    if( !m_thread_info )
    {
        m_thread_info.reset( new AsThreadInfo() );
    }
    if( !m_thread_info->native_task )
    {
        m_thread_info->native_task.reset( new Task(m_thread_info.get()) );
        LOG << "make_native_task: created native task " << m_thread_info->native_task.get() << "\n";
    }
}
void Scheduler::schedule_waiting_task()
{
    instance().do_schedule_waiting_task();
}

void Scheduler::do_schedule_waiting_task()
{
    /* At this point current task inserted in wait list and MUST not be inserted in
     * running list and current_task->m_context == nullptr to protect scheduling current task
     * for other thread (if it is AlterNative Task).
     */
    LOG << "Scheduler::wait_on: trying to schedule next task\n";
    bool switched = schedule(false);
    // Nothing to schedule or waiting finished
    Task* current_task = get_current_task();
    if( current_task->is_native() )
    {
        std::unique_lock<std::mutex> native_guard(current_task->m_native_info->native_mutex);
        while(current_task->m_state == TaskState::Waiting)
        {
            current_task->m_native_info->native_ready.wait(native_guard); // FIXME: limit wait time
        }
        return;
    }
    else
    {
        // if Task switched and then got back here it's Running, not Waiting
        // but if not we will switch to native without storing it running queue
        if( !switched )
        {
            switch_to(m_thread_info->native_task.get(), TaskState::Waiting);
        }
        return;
    }
}

Task* Scheduler::get_next_from_queue() noexcept
{
    bool have_more_tasks = false;
    Task* task = instance().running_queue_.get_task(have_more_tasks);
    LOG << "Scheduler::get_next_from_queue: got task " << task << " from running queue\n";
    if( task != nullptr
            && have_more_tasks)
    {
        instance().bg_runner_.notify();
    }
    return task;
}

Task* Scheduler::get_next_from_native()
{
    ::std::lock_guard<::std::mutex> native_guard(m_thread_info->native_mutex);
    if( m_thread_info->native_task->m_state == TaskState::Running )
    {
        LOG << "Scheduler::_get_next_from_native: got native task\n";
        return m_thread_info->native_task.get();
    }
    return nullptr;
}

void Scheduler::enqueue_task(Task *task) noexcept
{
    assert(task != nullptr);
    instance().running_queue_.put_task(task);
    LOG << "Scheduler::enqueue_task: task " << task << " stored in running task queue\n";
    instance().bg_runner_.notify();
}

Task *Scheduler::get_next_task()
{
    Task* current = get_current_task();
    Task* next_task = nullptr;
    if( current->is_native() ) // Native thread in Native code calls yield()
    {
        LOG << "Scheduler::get_next_task: in Native\n";
        return instance().get_next_from_queue();
    }
    else
    {
        if( m_thread_info->native_runner ) // Native thread running on AlterStack
        {
            LOG << "Scheduler::_get_next_task: in AlterNative\n";
            next_task = get_next_from_native();
            if( next_task != nullptr )
            {
                return next_task;
            }
            return instance().get_next_from_queue();
        }
        else // BgRunner on Alternative stack
        {
            LOG << "Scheduler::_get_next_task: in BgRunner\n";
            return instance().get_next_from_queue();
        }
    }
    return nullptr;
}

}
