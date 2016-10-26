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

#include <memory>

#include "futex.h"

namespace alterstack
{
class Task;

enum class RunnerType
{
    NativeRunner,
    BgRunner
};

struct RunnerInfo
{
    RunnerInfo() = delete;
    RunnerInfo( RunnerType type = RunnerType::NativeRunner );

    ::std::unique_ptr<Task>   native_task;
    Task* current_task = nullptr;
    Futex native_futex;
    RunnerType type;
};

inline RunnerInfo::RunnerInfo( RunnerType runner_type )
    :type( runner_type )
{}

}