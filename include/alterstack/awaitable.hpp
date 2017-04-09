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

#include <atomic>

namespace alterstack
{
class TaskBase;

/**
 * @brief Provides core functionality for one Task to wait another.
 *
 * When created Awaitable in active state and all wait() call will stop current Task and
 * store it in wait queue for this Awaitable. Current OS thread will switch to next
 * running task (if avalable).
 * If no Task avalable Native Task will wait on conditional_variable, AlterNative Task
 * will switch to Native (and sleep there).
 *
 * release() will transit Awaitable to finished state and wakeup all Task in wait queue.
 * Native will be marked Running and call it's conditional_variable notify(), AlterNative
 * marked as Running too and inserted in running Task queue.
 *
 * Destructor ~Awaitable() will call wait() if *this still active
 * and have nonempty wait queue.
 *
 * In finished state (after release() called) wait() and release() do nothing.
 *
 * wait() and release() are both threadsafe
 */
class Awaitable
{
public:
    Awaitable();
    /**
       @brief destructor will wait() if there are Tasks in wait queue
     */
    ~Awaitable();
    /**
     * @brief stop current Task until Awaitable will be released
     *
     * If Awaitable finished -> return immediately.
     * Else add current task in waiters list and switch OS thread to next running Task
     * or wait on conditional_variable if no Task in running queue.
     *
     * Returns when Awaitable::release() will be called.
     *
     * wait() is threadsafe
     */
    void wait();
    /**
     * @brief wakeup Tasks waiting on this Awaitable.
     *
     * If Awaitable finished -> do nothing.
     * Else mark it finished and release all Tasks waiting on this Awaitable.
     * Native Tasks will be notified.
     * AlterNative Tasks will be enqueued in running queue.
     *
     * release() is threadsafe
     */
    void release();

    struct AwaitableData
    {
        AwaitableData() noexcept
            :head{nullptr}
            ,is_finished{false}
        {}
        TaskBase* head;
        bool  is_finished;
    };
private:
    /**
     * @brief insert current task in wait list
     *
     * If this Awaitable still waited current task will be inserted in wait list,
     * current_task->m_context = nullptr;
     * and function returns true
     *
     * If this Awaitable got finished before function complete insert_current_task_in_waitlist()
     * do nothing and returns false
     * @return true if this still waited or false if finished
     */
    bool insert_current_task_in_waitlist();
    ::std::atomic<AwaitableData> m_data;
};

inline Awaitable::Awaitable()
    :m_data{}
{}

}
