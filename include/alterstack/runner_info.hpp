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

#include <memory>

#include "futex.hpp"
#include "task.hpp"

namespace alterstack
{

enum class RunnerType
{
    NativeRunner,
    BgRunner
};

class RunnerInfo
{
public:
    RunnerInfo();

    static RunnerInfo& current();
    RunnerType type() const;
    void set_type(RunnerType runner_type);

    Task* current_task = nullptr;
    Task  native_task;
    Futex native_futex;
    RunnerType m_type;
};

inline RunnerInfo::RunnerInfo()
    :native_task( this )
    ,m_type( RunnerType::NativeRunner )
{}

inline RunnerInfo& RunnerInfo::current()
{
    static thread_local RunnerInfo runner_info{};
    return runner_info;
}

inline RunnerType RunnerInfo::type() const
{
    return m_type;
}

inline void RunnerInfo::set_type(RunnerType runner_type)
{
    m_type = runner_type;
}

}
