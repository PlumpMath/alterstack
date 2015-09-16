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

#pragma once

#include "Awaitable.hpp"
#include "Context.hpp"
#include "Stack.hpp"
#include "NativeInfo.hpp"
#include "Task.hpp"
#include "Logger.hpp"
#include "TaskQueue.hpp"

#include <condition_variable>
#include <mutex>
#include <deque>
#include <atomic>
#include <cstdint>
#include <cassert>

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
public:
    /**
     * @brief schedule next task on current OS thread
     *
     * If current_still_running == true, current task stay Running
     *
     * schedule() tries to find next Running Task using scheduling algorithm of Scheduler.
     * If new task available switch OS thread to it. If no Task available
     * do nothing (continue running current Task).
     * @param old_stay_running if true, save old task in running queue
     * @return true if at least one switch done
     */
    static bool schedule(bool old_stay_running=true);
    /**
     * @brief switch OS thread to newly created Task, current task stay Running
     * @param task new task to run
     */
    static void schedule_new_task(Task *task);
    /**
     * @brief switch to new task or wait because current task is waiting
     */
    static void schedule_waiting_task();
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
    static void switch_to(Task* new_task, TaskState old_task_state=TaskState::Running);
    /**
     * @brief store old task in running queue, if it is not nullptr and is AlterNative
     * @param old_task task to store
     */
    static void post_switch_fixup(Task *old_task);

private:
    friend class Task;
    friend class Awaitable;
    friend class BgRunner;
    friend class CpuCore;

    /**
     * @brief get Task* from running queue
     * @return Task* or nullptr if queue is empty
     */
    static Task* get_next_from_queue() noexcept;
    /**
     * @brief get Native Task* if it is running or nullptr
     * @return Native Task* or nullptr
     */
    static Task* get_next_from_native();
    /**
     * @brief enqueue task in running queue
     * @param task task to store
     * get_next_from_native() is threadsafe
     */
    static void  enqueue_task(Task* task) noexcept;
    /**
     * @brief get current Task*
     * @return pointer to current Task
     */
    static Task* get_current_task();
    /**
     * @brief get Native Task pointer (create Task instance if does not exist)
     * @return Task* to Native Task instance
     */
    static Task* get_native_task();
    /**
     * @brief create thread_local native Task instance
     */
    static void  create_native_task();
    /**
     * @brief get next Task* to run using schedule algorithm of Scheduler
     * @return next running Task* or nullptr
     */
    static Task* get_next_task();
    /**
     * @brief wake up one BgRunner (if there is sleeping one)
     */
    static void  wakeup_bg_runner() noexcept;
    /**
     * @brief get running queue tasks list
     * @return task list
     */
    static Task* get_task_list() noexcept;

    static thread_local ::std::unique_ptr<AsThreadInfo> m_thread_info;

    // running Tasks queue data
    static ::std::mutex              m_queue_mutex;
    //static ::std::deque<Task*>       m_running_queue; // FIXME: make running queue lockfree
    static TaskQueue                 m_task_queue;
    static ::std::condition_variable m_task_ready;
};

inline Task *Scheduler::get_task_list() noexcept
{
    LOG << "Scheduler::get_task_list\n";
    return m_task_queue.get_all();
}

}
