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
#include <iostream>
#include <vector>
#include <set>

#include <catch.hpp>

#include "alterstack/task_stack.hpp"
#include "alterstack/intrusive_list.hpp"

namespace alterstack
{
class Task : public IntrusiveList<Task>
{
private:
    friend class TaskStack<Task>;
    friend class UnitTestAccessor;
};

class UnitTestAccessor
{
public:
    static alterstack::Task* get_next(alterstack::Task* task)
    {
        return task->next();
    }
};
}

using alterstack::TaskStack;
using alterstack::Task;

TEST_CASE("API check")
{
    TaskStack<Task> stack;
    SECTION( "empty TaskStack returns nullptr" )
    {
        for(int i = 0; i < 16; ++i )
        {
            REQUIRE( stack.pop_all() == nullptr );
        }
    }
    SECTION( "first push returns true, next false, push after pop_all returns true" )
    {
        std::vector<alterstack::Task> tasks(100);
        bool res = stack.push(&tasks[0]);
        REQUIRE( res == true );
        for( unsigned i = 1; i < tasks.size(); ++i)
        {
            res = stack.push(&tasks[i]);
            REQUIRE( res == false );
        }
        stack.pop_all();
        tasks.clear();
        tasks.resize(100);
        res = stack.push(&tasks[0]);
        REQUIRE( res == true );
        for( unsigned i = 1; i < tasks.size(); ++i)
        {
            res = stack.push(&tasks[i]);
            REQUIRE( res == false );
        }
    }
    SECTION( "filled by one element returns it" )
    {
        alterstack::Task task;
        stack.push(&task);
        alterstack::Task* got_task = stack.pop_all();
        REQUIRE( got_task == &task );
        REQUIRE( alterstack::UnitTestAccessor::get_next(got_task) == nullptr );
        SECTION( "and after that returns nullptr" )
        {
            for(int i = 0; i < 100; ++i )
            {
                REQUIRE( stack.pop_all() == nullptr );
            }
        }
    }
    constexpr int TASKS_COUNT = 100;
    std::vector<alterstack::Task> tasks(TASKS_COUNT);
    SECTION( "pop_all returns the same set of tasks, that push store" )
    {
        for(auto& task: tasks)
        {
            stack.push(&task);
        }
        alterstack::Task* task_list = stack.pop_all();
        std::set<alterstack::Task*> task_set;
        do
        {
            task_set.insert(task_list);
        }
        while( (task_list = alterstack::UnitTestAccessor::get_next(task_list))
               != nullptr );
        REQUIRE( task_set.size() == TASKS_COUNT );
        for(auto& task: tasks)
        {
            REQUIRE( task_set.find(&task) != task_set.end() );
            task_set.erase(&task);
        }
    }
}

