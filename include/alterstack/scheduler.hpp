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

#pragma once

#include "lock_free_queue.hpp"
#include "task_runner.hpp"
#include "bg_runner.hpp"
#include "passkey.hpp"

namespace alterstack
{
class TaskBase;
using RunningQueue = LockFreeQueue<TaskBase>;
/**
 * @brief Tasks scheduler.
 *
 * Implements tasks running queue and switching OS threads to next task or
 * waiting on conditional variable if there are no running task.
 *
 * For scheduling aggorithm see Main Page in section @ref scheduling_algorithm.
 */
class Scheduler
{
private:
    Scheduler();
    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

public:
    static bool schedule( TaskBase* current_task = get_current_task() );
    static void run_new_task( TaskBase *task );

    static void post_jump_fcontext( Passkey<Task>
                                    , ::scontext::transfer_t transfer
                                    , TaskBase* current_task );
    static void add_waiting_list_to_running( Passkey<Awaitable>, TaskBase* task_list ) noexcept;

private:
    static Scheduler& instance();

    bool do_schedule( TaskBase* current_task );
    void do_schedule_new_task(TaskBase *task);
    static void switch_to(TaskBase* new_task );
    static void post_jump_fcontext( ::scontext::transfer_t transfer
                                    ,TaskBase* current_task );

    TaskBase* get_next_task( TaskBase* current_task );
    TaskBase* get_running_from_queue() noexcept;
    static TaskBase* get_running_from_native();
    static TaskBase* get_native_task();
    static TaskBase* get_current_task();

    static void add_waiting_list_to_running( TaskBase* task_list ) noexcept;
    static void enqueue_unbound_task(TaskBase* task) noexcept;
    static void wait_while_context_is_null( std::atomic<Context>* context ) noexcept;

    BgRunner     bg_runner_;
    RunningQueue running_queue_;

private:
    friend class TaskBase;
    friend class BoundTask;
    friend class Task;
    friend class Awaitable;
    friend class BgRunner;
    friend class BgThread;
};

inline void Scheduler::post_jump_fcontext( Passkey<Task>
                                           , scontext::transfer_t transfer
                                           , TaskBase* current_task )
{
    post_jump_fcontext( transfer, current_task );
}

inline void Scheduler::add_waiting_list_to_running( Passkey<Awaitable>, TaskBase* task_list ) noexcept
{
    add_waiting_list_to_running( task_list);
}

/**
 * @brief get Scheduler instance singleton
 * @return Scheduler& singleton instance
 */
inline Scheduler& Scheduler::instance()
{
    static Scheduler scheduler;
    return scheduler;
}

}
