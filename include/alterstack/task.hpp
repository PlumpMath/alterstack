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
class TaskBase : private IntrusiveList<TaskBase>
{
private:
    TaskBase( bool is_thread_bound );

public:
    virtual ~TaskBase();

    TaskBase() = delete;
    TaskBase(const TaskBase&) = delete;
    TaskBase(TaskBase&&)      = delete;
    TaskBase& operator=(const TaskBase&) = delete;
    TaskBase& operator=(TaskBase&&)      = delete;

    void join();

    TaskState state( Passkey<Scheduler> ) const noexcept;

protected:
    bool is_thread_bound() const noexcept;
    TaskState state() const noexcept;
    void release();

    Awaitable              m_awaitable;
    // m_context == nullptr when some thread running this context
    std::atomic<Context>   m_context = { nullptr };
    std::atomic<TaskState> m_state   = { TaskState::Running };
    const bool m_is_thread_bound;

private:
    friend class Scheduler;
    friend class Awaitable;
    friend class BoundBuffer<TaskBase>;
    friend class LockFreeStack<TaskBase>;
    friend LockFreeQueue<TaskBase>;
    friend class UnitTestAccessor;
    friend class BgRunner;
    friend class Task;
    friend class BoundTask;
};

class Task final : public TaskBase
{
public:
    enum class Priority : uint32_t
    {
        High = 0,
        Normal = 1,
        Low = 2,
        Batch = 3,
        MaxNum = Batch
    };

    Task( ::std::function<void()> runnable ); ///< will create unbound Task
    ~Task();

    static void yield();
    Priority priority();
    void     set_priority( Priority prio );

private:
    [[noreturn]]
    static void _run_wrapper( ::scontext::transfer_t transfer ) noexcept;

    Priority                m_priority = { Priority::Normal }; ///< scheduling priority
    std::unique_ptr<Stack>  m_stack;
    ::std::function<void()> m_runnable;
};

inline Task::Priority Task::priority()
{
    return m_priority;
}

inline void Task::set_priority( Task::Priority prio )
{
    m_priority = prio;
}

class BoundTask final : public TaskBase
{
public:
    BoundTask( Passkey<TaskRunner>, TaskRunner* runner ); ///< will create thread bound Task
    ~BoundTask();
    void notify();
private:
    TaskRunner* m_task_runner;
};
inline TaskState TaskBase::state(Passkey<Scheduler>) const noexcept
{
    return state();
}
inline TaskState TaskBase::state() const noexcept
{
    return m_state.load( std::memory_order_acquire );
}

/**
 * @brief release all Tasks join()'ed this Task.
 *
 * release() is threadsafe
 */
inline void TaskBase::release()
{
    m_awaitable.release();
}

inline bool TaskBase::is_thread_bound() const noexcept
{
    return m_is_thread_bound;
}

}
