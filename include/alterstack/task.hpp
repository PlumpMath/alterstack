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

#include "intrusive_list.hpp"
#include "awaitable.hpp"
#include "stack.hpp"
#include "context.hpp"
#include "passkey.hpp"

namespace alterstack
{

enum class TaskState
{
    Running,
    Waiting,
    Finished,
};

class TaskRunner;
class Scheduler;
class Awaitable;
template<typename Task>
class BoundBuffer;
template<typename Task>
class LockFreeStack;
template<typename Task>
class LockFreeQueue;
/**
 * @brief Main class to start and wait tasks.
 *
 * Details see on Main Page in section @ref task_section.
 */
class Task : private IntrusiveList<Task>
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
    void join();

    TaskState state( Passkey<Scheduler> ) const noexcept;

private:
    /**
     * @brief helper function to start Task's runnable object and clean when it's finished
     * @param task_ptr pointer to Task instance
     */
    [[noreturn]]
    static void _run_wrapper( ::scontext::transfer_t transfer ) noexcept;
    bool is_thread_bound() noexcept;
    TaskState state() const noexcept;
    void release();

    Awaitable              m_awaitable;
    std::atomic<Context>   m_context; // this always == nullptr, when some thread running this context
    std::atomic<TaskState> m_state;

    std::unique_ptr<Stack> m_stack;
    ::std::function<void()> m_runnable;
    bool m_is_thread_bound;
private:
    friend class Scheduler;
    friend class Awaitable;
    friend class BoundBuffer<Task>;
    friend class LockFreeStack<Task>;
    friend LockFreeQueue<Task>;
    friend class UnitTestAccessor;
    friend class BgRunner;
};

inline TaskState Task::state(Passkey<Scheduler>) const noexcept
{
    return state();
}
inline TaskState Task::state() const noexcept
{
    return m_state.load( std::memory_order_acquire );
}

/**
 * @brief release all Tasks join()'ed this Task.
 *
 * release() is threadsafe
 */
inline void Task::release()
{
    m_awaitable.release();
}

inline bool Task::is_thread_bound() noexcept
{
    return m_is_thread_bound;
}

}
