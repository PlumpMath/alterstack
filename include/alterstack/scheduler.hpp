/*
 * Copyright 2015-2016 Alexey Syrnikov <san@masterspline.net>
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

#include <mutex>
#include <deque>
#include <atomic>
#include <cstdint>
#include <cassert>
#include <condition_variable>

#include "running_queue.hpp"
#include "awaitable.hpp"
#include "context.hpp"
#include "stack.hpp"
#include "task.hpp"
#include "bg_runner.hpp"
#include "logger.hpp"

namespace alterstack
{
class Task;

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

    static bool schedule(bool old_stay_running=true);
    static void schedule_new_task(Task *task);
    static void schedule_waiting_task();
    static void switch_to(Task* new_task, TaskState old_task_state=TaskState::Running);
    static void post_switch_fixup(Task *old_task);

private:
    friend class Task;
    friend class Awaitable;
    friend class BgRunner;
    friend class BgThread;

    bool do_schedule(bool old_stay_running);
    void do_schedule_new_task(Task *task);
    void do_schedule_waiting_task();

    static Scheduler& instance();

    Task* get_next_from_queue() noexcept;
    static Task* get_next_from_native();
    static void  enqueue_task(Task* task) noexcept;
    static Task* get_current_task();
    static Task* get_native_task();
    static Task* get_next_task();

    BgRunner bg_runner_;
    RunningQueue<Task> running_queue_;
};

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
