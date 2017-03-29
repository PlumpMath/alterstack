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

#include <functional>
#include <cstdint>
#include <memory>
#include <atomic>

#include "awaitable.hpp"
#include "TaskState.hpp"
#include "scheduler.hpp"
#include "stack.hpp"
#include "context.hpp"
#include "task_buffer.hpp"
#include "task_stack.hpp"
#include "running_queue.hpp"

namespace alterstack
{
class RunnerInfo;
/**
 * @brief Main class to start and wait tasks.
 *
 * Details see on Main Page in section @ref task_section.
 */
class Task
{
public:
    Task(); ///< will create unbound Task
    explicit Task(RunnerInfo *native_info); ///< will create create thread bound Task
    ~Task();

    Task(const Task&) = delete;
    Task(Task&&)      = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&)      = delete;

    void set_function();
    void run(::std::function<void()> runnable);
    static void yield();
    void wait();
    void release();

private:
    /**
     * @brief helper function to start Task's runnable object and clean when it's finished
     * @param task_ptr pointer to Task instance
     */
    [[noreturn]]
    static void _run_wrapper( ::scontext::transfer_t transfer ) noexcept;
    bool is_native() noexcept;

    Awaitable  awaitable_;
    Task*      next_ = nullptr;
    Context    m_context;

    RunnerInfo* const m_native_info;
    std::atomic<TaskState>  m_state;

    std::unique_ptr<Stack>  m_stack;
    ::std::function<void()> m_runnable;
private:
    friend class Scheduler;
    friend class Awaitable; // for manipulating m_next intrusive list pointer
    friend class TaskBuffer<Task>;
    friend class TaskStack<Task>;
    friend class RunningQueue<Task>;
    friend class UnitTestAccessor;
    friend class BgRunner;
};

/**
 * @brief release all Tasks waiting this.
 *
 * release() is threadsafe
 */
inline void Task::release()
{
    awaitable_.release();
}

inline bool Task::is_native() noexcept
{
    return m_native_info != nullptr;
}

}
