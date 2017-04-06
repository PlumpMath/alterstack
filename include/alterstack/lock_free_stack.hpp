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

#include "intrusive_list.hpp"
#include "bound_buffer.hpp"

namespace alterstack
{
/**
 * @brief Lockfree stack for T* items with push and pop_all operations (without pop single item)
 *
 * Items stored in intrusive list (so T* MUST have next() and set_next() methods)
 *
 * push(T*) will store one T* item in stack
 *
 * pop_all() will return all stored T* and clean stack atomically
 */
template<typename T>
class LockFreeStack
{
public:
    LockFreeStack() noexcept;

    LockFreeStack( const LockFreeStack& ) = delete;
    LockFreeStack( LockFreeStack&& )      = delete;
    LockFreeStack& operator=( const LockFreeStack& ) = delete;
    LockFreeStack& operator=( LockFreeStack&& )      = delete;

    bool push( T* item ) noexcept;
    T* pop_all() noexcept;

private:
    std::atomic<T*> m_head;
};

template<typename T>
LockFreeStack<T>::LockFreeStack() noexcept
{
    m_head.store( nullptr,std::memory_order_relaxed );
}

/**
 * @brief push single T* item (not list) in stack
 * @param T* item to store in stack
 * @return true if stack was empty
 */
template<typename T>
bool LockFreeStack<T>::push( T *item ) noexcept
{
    assert( item->next() == nullptr);
    T* head = m_head.load(std::memory_order_acquire);
    item->set_next( head );
    while( !m_head.compare_exchange_weak(
               head, item
               ,std::memory_order_release
               ,std::memory_order_relaxed) )
    {
        item->set_next( head );
    }
    return (head == nullptr) ? true : false;
}

/**
 * @brief atomically get whole T* items list from stack and clear stack
 * @return T* items list
 */
template<typename T>
T* LockFreeStack<T>::pop_all() noexcept
{
    return m_head.exchange( nullptr, std::memory_order_acquire );
}

}
