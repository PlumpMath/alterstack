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

#include "bound_buffer.hpp"
#include "task_stack.hpp"

namespace alterstack
{
/**
 * @brief Task Queue ready to run.
 *
 * Task* get_task() noexcept; dequeue single Task* from queue if exists
 * or return nullptr
 *
 * void put_task(Task* task) noexcept; enqueue single Task* in queue
 */
template<typename Task>
class RunningQueue
{
public:
    RunningQueue() = default;
    /**
     * @brief dequeue Task* or nullptr
     *
     * Dequeue Task* and set flag if there is more stored Task. Sometimes flag
     * can be not set even queue has more stored Task* (false negative). Will
     * never store false in this flag.
     * @param have_more_tasks will set this flag to true, if there is more tasks
     * @return single Task* (not list) or nullptr if no Task* in queue
     */
    Task* get_task(bool &have_more_tasks) noexcept;
    /**
     * @brief enqueue single Task* in queue
     *
     * @param task Task* to store
     */
    void put_task(Task* task) noexcept;

private:
    BoundBuffer<Task> task_buffer_;
    TaskStack<Task>  task_stack_;
};

template<typename Task>
Task *RunningQueue<Task>::get_task(bool& have_more_tasks) noexcept
{
    Task* task = task_buffer_.get_item( have_more_tasks );
    if( task != nullptr )
    {
        return task;
    }
    Task* task_list = task_stack_.pop_all();
    if( task_list != nullptr )
    {
        task = task_list;
        task_list = task_list->next();
        task->set_next( nullptr );
        if( task_list != nullptr )
        {
            task_buffer_.put_items_list(task_list);
            have_more_tasks = true;
        }
    }
    return task;
}

template<typename Task>
void RunningQueue<Task>::put_task(Task *task) noexcept
{
    if( task_stack_.push(task) )
    {
        // FIXME: notify BgRunner here (first task in stack)
    }
}

}
