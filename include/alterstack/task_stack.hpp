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
#pragma once

#include <assert.h>
#include <atomic>

#include "task_buffer.hpp"

namespace alterstack
{
/**
 * @brief Lockfree stack for Task* with push and pop_all operations (without pop)
 *
 * push(Task*) will store one Task* in stack
 *
 * pop_all() will return all stored Task* and clean stack atomically
 */
template<typename Task>
class TaskStack
{
public:
    TaskStack() noexcept;
    /**
     * @brief push single Task* (not list) in stack
     * @param task Task* to store in stack
     * @return true if stack was empty
     */
    bool push(Task* task) noexcept;
    /**
     * @brief atomically get whole Task* list from stack and empty it
     * @return Task* list
     */
    Task* pop_all() noexcept;
private:
    std::atomic<Task*> head_;
};
template<typename Task>
TaskStack<Task>::TaskStack() noexcept
{
    head_.store(nullptr,std::memory_order_relaxed);
}
template<typename Task>
bool TaskStack<Task>::push(Task *task) noexcept
{
    assert( task->next() == nullptr);
    Task* head = head_.load(std::memory_order_acquire);
    task->set_next( head );
    while( !head_.compare_exchange_weak(
               head,task
               ,std::memory_order_release
               ,std::memory_order_relaxed) )
    {
        task->set_next( head );
    }
    return (head == nullptr) ? true : false;
}
template<typename Task>
Task* TaskStack<Task>::pop_all() noexcept
{
    return head_.exchange(nullptr, std::memory_order_acquire);
}

}
