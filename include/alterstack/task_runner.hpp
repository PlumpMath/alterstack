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

#include <memory>

#include "futex.hpp"
#include "task.hpp"
#include "passkey.hpp"

namespace alterstack
{

enum class RunnerType
{
    CommonThread, ///< main thread or some thread with user's code
    BgRunner,     ///< special BgRunner thread
};
class BgThread;

class TaskRunner
{
public:
    TaskRunner( const TaskRunner& ) = delete;
    TaskRunner( TaskRunner&& )      = delete;
    TaskRunner& operator=( const TaskRunner& ) = delete;
    TaskRunner& operator=( TaskRunner&& )      = delete;

    static TaskRunner& current();
    static Task* current_task();
    static void  set_task( Task* new_task );
    static Task* native_task();
    RunnerType   type();

    void make_bg_runner( Passkey<BgThread> );

    Futex native_futex;
private:
    TaskRunner();
    void set_type( RunnerType type );

    Task  m_native_task;
    Task* m_current_task = nullptr;
    RunnerType m_runner_type;
};

inline TaskRunner::TaskRunner()
    :m_native_task( Passkey<TaskRunner>{} )
{}

inline TaskRunner& TaskRunner::current()
{
    static thread_local TaskRunner runner_info{};
    return runner_info;
}

inline Task* TaskRunner::current_task()
{
    return current().m_current_task;
}

inline void TaskRunner::set_task(Task *new_task)
{
    current().m_current_task = new_task;
}

inline Task* TaskRunner::native_task()
{
    return &current().m_native_task;
}

inline RunnerType TaskRunner::type()
{
    return m_runner_type;
}

inline void TaskRunner::make_bg_runner( Passkey<BgThread> )
{
    set_type( RunnerType::BgRunner );
}

inline void TaskRunner::set_type(RunnerType type)
{
    m_runner_type = type;
}

}
