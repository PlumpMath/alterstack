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

#include <condition_variable>
#include <mutex>

namespace alterstack
{
class Task;

enum class RunnerType
{
    NativeRunner,
    BgRunner
};

struct AsThreadInfo
{
    AsThreadInfo() = delete;
    AsThreadInfo( RunnerType runner_type = RunnerType::NativeRunner );

    ::std::unique_ptr<Task>   native_task;
    Task*                     current_task = nullptr;
    ::std::mutex              native_mutex;
    ::std::condition_variable native_ready;
    /// used to distinguish AlterNative Task runner thread Native vs. BgRunner
    RunnerType                runner_type;
};

inline AsThreadInfo::AsThreadInfo( RunnerType runner_type )
    :runner_type( runner_type )
{}

}
