/*
 * Copyright 2015,2017 Alexey Syrnikov <san@masterspline.net>
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

#include <algorithm>
#include <thread>
#include <iostream>
#include <vector>
#include <set>

#include <catch.hpp>

#include "alterstack/running_queue.hpp"

namespace alterstack
{
class Task
{
private:
    friend class BoundBuffer<Task>;
    friend class TaskStack<Task>;
    friend class RunningQueue<Task>;
    friend class UnitTestAccessor;
    Task* next_ = nullptr;
};

class UnitTestAccessor
{
public:
    static alterstack::Task* get_next(alterstack::Task* task)
    {
        return task->next_;
    }
};
}

using alterstack::RunningQueue;
using alterstack::Task;

RunningQueue<Task> queue;
constexpr int TASKS_COUNT = 1000;
std::vector<Task> tasks(TASKS_COUNT);

void thread_function()
{
    for(int i = 0; i < 10000; ++i)
    {
        alterstack::Task* task;
        std::set<alterstack::Task*> task_set;
        bool have_more_tasks = false;
        while( (task = queue.get_task(have_more_tasks)) != nullptr )
        {
            if( alterstack::UnitTestAccessor::get_next(task) != nullptr )
            {
                std::cerr << "got not single Task*\n";
                std::cerr << "task " << task << "\n";
                std::cerr << "next " << alterstack::UnitTestAccessor::get_next(task) << "\n";
                exit(1);
            }
            task_set.insert(task);
        }
//        for(auto& task: tasks)
//        {
//            if( task_set.find(&task) == task_set.end() )
//            {
//                std::cerr << "Got task " << &task << " not from initial array\n";
//                exit(1);
//            }
//        }
        for( auto& task: task_set)
        {
            queue.put_task(task);
        }
        task_set.clear();
    }
}

int main()
{
    if( intptr_t(&queue) % 64 != 0 )
    {
        std::cerr << "Buffer unaligned " << &queue << "\n";
    }
    for(auto& task: tasks)
    {
        queue.put_task(&task);
    }

    std::thread t1(thread_function);
    std::thread t2(thread_function);
    t1.join();
    t2.join();
    alterstack::Task* task;
    std::set<alterstack::Task*> task_set;
    bool have_more_tasks = false;
    while( (task = queue.get_task(have_more_tasks)) != nullptr )
    {
        if( alterstack::UnitTestAccessor::get_next(task) != nullptr )
        {
            std::cerr << "got not single Task*\n";
            std::cerr << "task " << task << "\n";
            std::cerr << "next " << alterstack::UnitTestAccessor::get_next(task) << "\n";
            exit(1);
        }
        task_set.insert(task);
    }
    if( task_set.size() != TASKS_COUNT )
    {
        std::cerr << "task_set.size() != TASKS_COUNT\n";
        exit(1);
    }
    for(auto& task: tasks)
    {
        if( task_set.find(&task) == task_set.end() )
        {
            std::cerr << "Got task " << &task << " not from initial array\n";
            exit(1);
        }
    }
}
