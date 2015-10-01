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

#include <functional>
#include <cstdint>
#include <memory>

#include "TaskState.hpp"
#include "Scheduler.hpp"
#include "Stack.hpp"
#include "Context.hpp"
#include "NativeInfo.hpp"

namespace alterstack
{
/**
 * @brief Main class to start and wait tasks.
 *
 * Details see on Main Page in section @ref task_section.
 *
 */
class Task : public Awaitable
{
public:
    /**
     * @brief constructor to create AlterNative Task
     */
    Task();
    /**
     * @brief constructor to create Native Task
     *
     * It is Hack
     * @param native_info pointer to NativeInfo
     */
    explicit Task(AsThreadInfo *native_info);
    /**
     * @brief destructor will wait if Task still Running
     */
    ~Task();
    Task(const Task&) = delete;
    Task(Task&&)      = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&)      = delete;

    /**
     * @brief temporary function (HACK)
     *
     * will be removed or modified when any runnable can be started by Task
     */
    void set_function();
    /**
     * @brief starts executing Task
     * @param runnable void() function or functor to start
     */
    void run(::std::function<void()> runnable);
    /**
     * @brief yield current Task, schedule next (if avalable), current stay running
     */
    static void yield();
    /**
     * @brief switch caller Task in Waiting state while this is Running
     *
     * If this already finished return immediately
     */
    void wait();

private:
    friend class Scheduler;
    friend class Awaitable; // for manipulating m_next intrusive list pointer
    friend class TaskQueue;
    friend class BgRunner;

    /**
     * @brief helper function to start Task's runnable object and clean when it's finished
     * @param task_ptr pointer to Task instance
     */
    static void _run_wrapper(intptr_t task_ptr) noexcept;
    bool is_native() noexcept;

    Task*      m_next = nullptr;
    Context    m_context;

    AsThreadInfo* const m_native_info;
    TaskState    m_state;

    std::unique_ptr<Stack>  m_stack;
    ::std::function<void()> m_runnable;
};

inline bool Task::is_native() noexcept
{
    return m_native_info != nullptr;
}

}
