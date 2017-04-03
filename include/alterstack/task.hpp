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
#include "stack.hpp"
#include "context.hpp"
#include "task_buffer.hpp"
#include "task_stack.hpp"
#include "running_queue.hpp"
#include "passkey.hpp"

namespace alterstack
{

enum class TaskState
{
    Running,
    Waiting,
    Finished, ///< Task function finished, but it's stack can still be in use, so it's temporary
              /// state to Clear
    Clear    ///< this state MUST be set (atomically) after context switch
             /// because Task destructor in some thread can exit in case TaskState == Clear
             /// and Task memory can be freed so nobody can read or write Task data in parallel
             /// thread if TaskState == Clear (Task stack also MUST be not used)
};

class TaskRunner;
class Scheduler;
/**
 * @brief Main class to start and wait tasks.
 *
 * Details see on Main Page in section @ref task_section.
 */
class Task
{
public:
    Task( ::std::function<void()> runnable ); ///< will create unbound Task
    // FIXME: make this in factory
    Task( Passkey<TaskRunner> ); ///< will create thread bound Task
    ~Task();

    Task() = delete;
    Task(const Task&) = delete;
    Task(Task&&)      = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&)      = delete;

    static void yield();
    void wait();
    void release();

    TaskState state(Passkey<Scheduler>) const;

private:
    /**
     * @brief helper function to start Task's runnable object and clean when it's finished
     * @param task_ptr pointer to Task instance
     */
    [[noreturn]]
    static void _run_wrapper( ::scontext::transfer_t transfer ) noexcept;
    bool is_thread_bound() noexcept;

    Awaitable  awaitable_;
    Task*      next_ = nullptr;
    Context    m_context;

    std::atomic<TaskState>  m_state;

    std::unique_ptr<Stack>  m_stack;
    ::std::function<void()> m_runnable;
    bool m_is_thread_bound;
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

inline TaskState Task::state(Passkey<Scheduler>) const
{
    return m_state.load( std::memory_order_acquire );
}

inline bool Task::is_thread_bound() noexcept
{
    return m_is_thread_bound;
}

}
