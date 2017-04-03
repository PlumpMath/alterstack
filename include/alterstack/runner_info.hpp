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
#include "scheduler.hpp"

namespace alterstack
{

class RunnerInfo
{
public:
    RunnerInfo();

    static RunnerInfo& current();
    static Task* current_task();
    static void  set_task( Task* new_task );
    static Task* native_task();

    Futex native_futex;
private:
    Task  m_native_task;
    Task* m_current_task = nullptr;
};

inline RunnerInfo::RunnerInfo()
    :m_native_task( this )
{}

inline RunnerInfo& RunnerInfo::current()
{
    static thread_local RunnerInfo runner_info{};
    return runner_info;
}

inline Task *RunnerInfo::current_task()
{
    return current().m_current_task;
}

inline void RunnerInfo::set_task(Task *new_task)
{
    current().m_current_task = new_task;
}

inline Task* RunnerInfo::native_task()
{
    return &current().m_native_task;
}

}
